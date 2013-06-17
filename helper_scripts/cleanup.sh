dmsetup -vvvv remove ubuntu32--ng-root_datto_org
dmsetup -vvvv remove ubuntu32--ng-root_datto_cow
dmsetup -vvvv remove ubuntu32--ng-root_datto_dup

dmsetup -y -vvvv udevcomplete_all
