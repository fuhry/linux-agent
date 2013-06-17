#define _GNU_SOURCE /* for basename, see man 3 basename */
#include <string.h>
#include <error.h>
#include <libdevmapper.h>
#include <stdio.h>
#include <stdlib.h>

#define DUP_POSTFIX "_datto_dup"
#define SNAP_POSTFIX "_datto_cow"
#define ORIGIN_POSTFIX "_datto_org"

#define SNAPSHOT_TARGET "snapshot"
#define SNAPSHOT_ORIGIN_TARGET "snapshot-origin"

#define NON_PERSISTENT "N"
#define CHUNK_SIZE "32"

/* Maintenance notes:
 *
 * - dm_task_set_cookie must be called right before dm_task_run * or any
 *   other failures will prevent the semaphore from being decremented.
 *
 * - style of this code is directly influenced from the dm_setup source
 *
 * - Call dm_udev_create_cookie at the beginning of a function, and if it
 *   fails return on the spot so we don't end up waiting on a bad semaphore.
 *
 */

static int _duplicate_table(const char *, const char *);
static int _create_snapshot(const char *, const char *, const char*);
static int _create_snapshot_origin(const char *, const char *);
static int _suspend(const char *);
static int _resume(const char *);
static int _remove(const char *);
static int _load_from_device(const char *, const char *);

static int _copy_targets_from_device(const char *, struct dm_task *);
static int _get_num_sectors(const char *, uint64_t *);

/* All of these functions malloc memory, caller must free */
static int _create_dup_name(const char *, char **);
static int _create_snap_name(const char *, char **);
static int _create_orig_name(const char *, char **);

int takedown_cow_device(const char *dm_device_path)
{
	int r = 0;

	int suspended = 0;

	char *dup_name = NULL;
	char *snap_name = NULL;
	char *orig_name = NULL;
	const char *dm_device_name = NULL;

	dm_device_name = basename(dm_device_path);

	if (!_create_dup_name(dm_device_name, &dup_name))
		goto out;

	if (!_suspend(dm_device_name))
		goto out;

	suspended = 1;

	if (!_load_from_device(dup_name, dm_device_name))
		goto out;

	if (!_resume(dm_device_name))
		goto out;

	suspended = 0;

	if (!_create_orig_name(dm_device_name, &orig_name))
		goto out;

	if (!_remove(orig_name))
		goto out;

	if (!_create_snap_name(dm_device_name, &snap_name))
		goto out;

	if (!_remove(snap_name))
		goto out;

	if (!_remove(dup_name))
		goto out;

	r = 1;

out:
	if (suspended) {
		if (!_resume(dm_device_name))
			/* TODO: Make this much, much louder */
			error(0, 0, "Error resuming device");
	}

	free(dup_name);
	free(snap_name);
	free(orig_name);

	return r;
}

int setup_cow_device(const char *dm_device_path, const char *mem_dev,
		char *cow_path)
{
	int r = 0;

	int suspended = 0;

	char *dup_name = NULL;
	char *snap_name = NULL;
	char *orig_name = NULL;
	const char *dm_device_name = NULL;
	const char *dm_dev_dir = NULL;

	dm_device_name = basename(dm_device_path);

	if (!_create_dup_name(dm_device_name, &dup_name))
		goto out;

	if (!_duplicate_table(dm_device_name, dup_name)) {
		error(0, 0, "Error creating duplicate table");
		goto out;
	}

	if (!_suspend(dm_device_name)) {
		error(0, 0, "Error suspend device");
		goto out;
	}

	/* Set this flag so we know to resume */
	suspended = 1;

	if (!_create_snap_name(dm_device_name, &snap_name))
		goto out;

	if (!_create_snapshot(dup_name, snap_name, mem_dev)) {
		error(0, 0, "Error creating snapshot");
		goto out;
	}

	if (!_create_orig_name(dm_device_name, &orig_name))
		goto out;

	if (!_create_snapshot_origin(dup_name, orig_name)) {
		error(0, 0, "Error creating snapshot-origin");
		goto out;
	}

	if (!_load_from_device(orig_name, dm_device_name)) {
		error(0, 0, "Error loading snapshot-origin into original device");
		goto out;
	}

	dm_dev_dir = dm_dir();
	strcpy(cow_path, dm_dev_dir);
	strcat(cow_path, "/");
	strcat(cow_path, snap_name);

	r = 1;

out:
	if (suspended) {
		if (!_resume(dm_device_name)) {
			/* TODO: Make this much, much louder */
			error(0, 0, "Error resuming device");
		}
	}

	free(dup_name);
	free(snap_name);
	free(orig_name);

	return r;
}

static int _create_dup_name(const char *dm_device_name, char **dup_name)
{
	*dup_name = malloc(strlen(dm_device_name) + strlen(DUP_POSTFIX) + 1);
	if (dup_name == NULL) {
		perror("malloc");
		return 0;
	}

	strcpy(*dup_name, dm_device_name);
	strcat(*dup_name, DUP_POSTFIX);

	return 1;
}
static int _create_snap_name(const char *dm_device_name, char **snap_name)
{
	*snap_name = malloc(strlen(dm_device_name) + strlen(SNAP_POSTFIX) + 1);
	if (*snap_name == NULL) {
		perror("malloc");
		return 0;
	}

	strcpy(*snap_name, dm_device_name);
	strcat(*snap_name, SNAP_POSTFIX);

	return 1;
}

static int _create_orig_name(const char *dm_device_name, char **orig_name)
{
	*orig_name = malloc(strlen(dm_device_name) + strlen(ORIGIN_POSTFIX) + 1);
	if (*orig_name == NULL) {
		perror("malloc");
		return 0;
	}

	strcpy(*orig_name, dm_device_name);
	strcat(*orig_name, ORIGIN_POSTFIX);
	return 1;
}

static int _duplicate_table(const char *dm_source, const char *dm_dest)
{
	int r = 0;

	uint32_t udev_cookie = 0;

	struct dm_task *dm_create_task = NULL;

	if (!dm_udev_create_cookie(&udev_cookie))
		return 0;

	if (!(dm_create_task = dm_task_create(DM_DEVICE_CREATE)))
		goto out;

	if (!dm_task_set_name(dm_create_task, dm_dest))
		goto out;

	if (!_copy_targets_from_device(dm_source, dm_create_task))
		goto out;

	if (!dm_task_set_cookie(dm_create_task, &udev_cookie, 0))
		goto out;

	if (!dm_task_run(dm_create_task))
		goto out;

	r = 1;

out:
	dm_task_destroy(dm_create_task);

	dm_udev_wait(udev_cookie);

	return r;
}

static int _copy_targets_from_device(const char *dm_source,
		struct dm_task *dm_dest_task)
{
	int r = 0;

	void *next = NULL;
	uint64_t start;
	uint64_t length;
	char *target_type = NULL;
	char *params = NULL;

	struct dm_task *dm_table_task = NULL;

	if (!(dm_table_task = dm_task_create(DM_DEVICE_TABLE)))
		goto out;

	if (!dm_task_set_name(dm_table_task, dm_source))
		goto out;

	if (!dm_task_run(dm_table_task))
		goto out;

	do {
		next = dm_get_next_target(dm_table_task, next, &start, &length,
				&target_type, &params);
		dm_task_add_target(dm_dest_task, start, length, target_type,
				params);
	} while(next);

	r = 1;

out:
	dm_task_destroy(dm_table_task);

	return r;
}

static int _create_snapshot(const char *dm_device_name,
		const char *dm_snap_name, const char *mem_dev)
{
	int r = 0;

	uint32_t udev_cookie = 0;

	uint64_t num_sectors;
	const char *dm_dev_dir = NULL;

	char *params = NULL;
	struct dm_task *dm_create_task = NULL;

	if (!dm_udev_create_cookie(&udev_cookie))
		goto out;


	if (!(dm_create_task = dm_task_create(DM_DEVICE_CREATE)))
		goto out;

	dm_dev_dir = dm_dir();

	/* Each param is separated by a space, and the device path has a
	 * '/' between the dm_dir and the device name */
	params = malloc(strlen(dm_dev_dir) + 1 +
			strlen(dm_device_name) + 1 +
			strlen(mem_dev) + 1 +
			strlen(NON_PERSISTENT) + 1 +
			strlen(CHUNK_SIZE) + 1);

	if (params == NULL) {
		perror("malloc");
		goto out;
	}

	strcpy(params, dm_dev_dir);
	strcat(params, "/");
	strcat(params, dm_device_name);
	strcat(params, " ");
	strcat(params, mem_dev);
	strcat(params, " ");
	strcat(params, NON_PERSISTENT);
	strcat(params, " ");
	strcat(params, CHUNK_SIZE);

	if (!_get_num_sectors(dm_device_name, &num_sectors))
		goto out;

	if (!dm_task_add_target(dm_create_task, 0, num_sectors, SNAPSHOT_TARGET, 
			params)) {
		goto out;
	}

	if (!dm_task_set_name(dm_create_task, dm_snap_name))
		goto out;

	if (!dm_task_set_cookie(dm_create_task, &udev_cookie, 0))
		goto out;

	if (!dm_task_run(dm_create_task))
		goto out;

	r = 1;

out:
	free(params);

	dm_task_destroy(dm_create_task);

	dm_udev_wait(udev_cookie);

	return r;
}

int _create_snapshot_origin(const char *dm_device_name,
		const char *dm_orig_name)
{
	int r = 0;

	uint64_t num_sectors;

	uint32_t udev_cookie;

	char *params = NULL;
	const char *dm_dev_dir = NULL;

	struct dm_task *dm_create_task = NULL;

	if (!dm_udev_create_cookie(&udev_cookie))
		goto out;

	if (!(dm_create_task = dm_task_create(DM_DEVICE_CREATE)))
		goto out;

	dm_dev_dir = dm_dir();

	/* dm_dir + / + name + \0 */
	params = malloc(strlen(dm_dev_dir) + 1 + strlen(dm_device_name) + 1);

	if (params == NULL) {
		perror("malloc");
		goto out;
	}

	strcpy(params, dm_dev_dir);
	strcat(params, "/");
	strcat(params, dm_device_name);

	if (!_get_num_sectors(dm_device_name, &num_sectors))
		goto out;

	if (!dm_task_add_target(dm_create_task, 0, num_sectors,
				SNAPSHOT_ORIGIN_TARGET, params)) {
		goto out;
	}

	if (!dm_task_set_name(dm_create_task, dm_orig_name))
		goto out;

	if (!dm_task_set_cookie(dm_create_task, &udev_cookie, 0))
		goto out;

	if (!dm_task_run(dm_create_task))
		goto out;

	r = 1;

out:
	free(params);

	dm_task_destroy(dm_create_task);

	dm_udev_wait(udev_cookie);

	return r;
}

static int _get_num_sectors(const char *dm_device, uint64_t *result)
{
	int r = 0;

	void *next = NULL;
	uint64_t start;
	uint64_t length;
	char *target_type = NULL;
	char *params = NULL;

	struct dm_task *dm_table_task = NULL;

	if (!(dm_table_task = dm_task_create(DM_DEVICE_TABLE)))
		goto out;

	if (!dm_task_set_name(dm_table_task, dm_device))
		goto out;

	if (!dm_task_run(dm_table_task))
		goto out;

	/* Number of sectors is the sum of the lengths */
	*result = 0;
	do {
		next = dm_get_next_target(dm_table_task, next, &start, &length,
				&target_type, &params);
		*result += length;
	} while (next);

	r = 1;

out:
	dm_task_destroy(dm_table_task);
	return r;
}

static int _load_from_device(const char *dm_source_name,
		const char *dm_dest_name)
{
	int r = 0;
	struct dm_task *dm_load_task = NULL;

	if (!(dm_load_task = dm_task_create(DM_DEVICE_RELOAD)))
		return 0;

	if (!dm_task_set_name(dm_load_task, dm_dest_name))
		goto out;

	if (!_copy_targets_from_device(dm_source_name, dm_load_task))
		goto out;

	if (!dm_task_run(dm_load_task))
		goto out;

	r = 1;

out:
	dm_task_destroy(dm_load_task);

	return r;
}

int _remove(const char *dm_device_name)
{
	int r = 0;
	struct dm_task *dm_remove_task = NULL;

	if (!(dm_remove_task = dm_task_create(DM_DEVICE_REMOVE)))
		return 0;

	if (!dm_task_set_name(dm_remove_task, dm_device_name))
		goto out;

	if (!dm_task_run(dm_remove_task))
		goto out;

	r = 1;

out:
	dm_task_destroy(dm_remove_task);

	return r;
}

int _suspend(const char *dm_device_name)
{
	int r = 0;
	struct dm_task *dm_suspend_task = NULL;

	if (!(dm_suspend_task = dm_task_create(DM_DEVICE_SUSPEND)))
		return 0;

	if (!dm_task_set_name(dm_suspend_task, dm_device_name))
		goto out;

	if (!dm_task_run(dm_suspend_task))
		goto out;

	r = 1;

out:
	dm_task_destroy(dm_suspend_task);

	return r;
}

static int _resume(const char *dm_device_name)
{
	int r = 0;
	struct dm_task *dm_resume_task;
	uint32_t udev_cookie = 0;

	if (!dm_udev_create_cookie(&udev_cookie))
		goto out;

	if (!(dm_resume_task = dm_task_create(DM_DEVICE_RESUME)))
		return 0;

	if (!dm_task_set_name(dm_resume_task, dm_device_name))
		goto out;

	if (!dm_task_set_cookie(dm_resume_task, &udev_cookie, 0))
		goto out;

	if (!dm_task_run(dm_resume_task))
		goto out;

	r = 1;

out:
	dm_task_destroy(dm_resume_task);

	dm_udev_wait(udev_cookie);

	return r;
}

