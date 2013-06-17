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

static int _get_num_sectors(const char *, uint64_t *);

int setup_cow_device(const char *dm_device_path, const char *mem_dev)
{
	int r = 0;
	char *dup_name = NULL;
	char *snap_name = NULL;
	char *orig_name = NULL;
	const char *dm_source = NULL;

	dm_source = basename(dm_device_path);

	dup_name = malloc(strlen(dm_source) + strlen(DUP_POSTFIX) + 1);
	if (dup_name == NULL) {
		perror("malloc");
		goto out;
	}

	strcpy(dup_name, dm_source);
	strcat(dup_name, DUP_POSTFIX);

	if (!_duplicate_table(dm_source, dup_name)) {
		error(0, 0, "Error creating duplicate table");
		goto out;
	}

	snap_name = malloc(strlen(dm_source) + strlen(SNAP_POSTFIX) + 1);
	if (snap_name == NULL) {
		perror("malloc");
		goto out;
	}

	strcpy(snap_name, dm_source);
	strcat(snap_name, SNAP_POSTFIX);

	if (!_create_snapshot(dup_name, snap_name, mem_dev)) {
		error(0, 0, "Error creating snapshot");
		goto out;
	}

	orig_name = malloc(strlen(dm_source) + strlen(ORIGIN_POSTFIX) + 1);
	if (orig_name == NULL) {
		perror("malloc");
		goto out;
	}

	strcpy(orig_name, dm_source);
	strcat(orig_name, ORIGIN_POSTFIX);

	if (!_create_snapshot_origin(dup_name, orig_name)) {
		error(0, 0, "Error creating snapshot-origin");
		goto out;
	}

	r = 1;

out:
	free(dup_name);
	free(snap_name);
	free(orig_name);

	return r;
}

static int _duplicate_table(const char *dm_source, const char *dm_dest)
{
	int r = 0;

	uint32_t udev_cookie = 0;

	void *next = NULL;
	uint64_t start;
	uint64_t length;
	char *target_type = NULL;
	char *params = NULL;

	struct dm_task *dm_create_task = NULL;
	struct dm_task *dm_table_task = NULL;

	if (!dm_udev_create_cookie(&udev_cookie))
		return 0;

	if (!(dm_table_task = dm_task_create(DM_DEVICE_TABLE)))
		goto out;

	if (!dm_task_set_name(dm_table_task, dm_source))
		goto out;

	if (!dm_task_run(dm_table_task))
		goto out;

	if (!(dm_create_task = dm_task_create(DM_DEVICE_CREATE)))
		goto out;

	/* Copy targets from table task to create task */
	do {
		next = dm_get_next_target(dm_table_task, next, &start, &length,
				&target_type, &params);
		dm_task_add_target(dm_create_task, start, length, target_type,
				params);
	} while(next);

	if (!dm_task_set_name(dm_create_task, dm_dest))
		goto out;

	if (!dm_task_set_cookie(dm_create_task, &udev_cookie, 0))
		goto out;

	if (!dm_task_run(dm_create_task))
		goto out;

	r = 1;

out:
	dm_task_destroy(dm_table_task);
	dm_task_destroy(dm_create_task);

	dm_udev_wait(udev_cookie);

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

	printf("%s\n", params);

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


int _get_info(const char *dm_device, struct dm_info *dm_info)
{
	int r = 0;
	struct dm_task *dm_info_task;

	if (!(dm_info_task = dm_task_create(DM_DEVICE_INFO)))
		return 0;

	if (!dm_task_set_name(dm_info_task, dm_device))
		goto out;

	if (!dm_task_run(dm_info_task))
		goto out;

	if (!dm_task_get_info(dm_info_task, dm_info))
		goto out;

	r = 1;

out:
	dm_task_destroy(dm_info_task);

	return r;
}
