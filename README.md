# ZNSwap

This is the kernel code for "ZNSwap: un-Block your Swap", Usenix ATC 22. Tested with Ubuntu 20.04.

This source-code is POC and given as-is. Code structure can be improved, otherwise it is entirely functional. Any suggestions are more than welcome!

## Install:

```bash
make ARCH=x86 znswap_defconfig
make
sudo make modules_install
sudo make install
```

## Setup environment:

Make sure the device is formatted with 64B metadata via the `nvme-cli` tool.

Turn off all swap devices in the system, but one at a time:

`sudo swapoff /dev/<swap_dev>`

Reset the zoned-device

`sudo nvme zns reset-zone -a /dev/<zoned_swap_dev>`

## Device configuration options:

To set the maximum amount of zones allowed to be consumed:

`echo <num_zones> | sudo tee  /sys/kernel/mm/zns_swap/nr_swap_zones`

## Swap policy configurations:

### Swap policy:

Select swap policy:
* Allocate all swap slots from a single open zone: `echo 0 | sudo tee /sys/kernel/mm/zns_swap/zns_policy`
* Multiplex zone allocation per-cpu: `echo 1 | sudo tee /sys/kernel/mm/zns_swap/zns_policy`
* Multiplex zone allocation PID: `echo 2 | sudo tee /sys/kernel/mm/zns_swap/zns_policy`
* Multiplex zone allocation per-VMA: `echo 3 | sudo tee /sys/kernel/mm/zns_swap/zns_policy`
* Zone allocation driven by page "hotness": `echo 4 | sudo tee /sys/kernel/mm/zns_swap/zns_policy`
* Multiplex zone allocation cgroups: `echo 5 | sudo tee /sys/kernel/mm/zns_swap/zns_policy`
* Custom policy via [loadable kernel module + API][1]: `echo 6 | sudo tee /sys/kernel/mm/zns_swap/zns_policy`

To enable GC accounting as part of per-cgroup BW consumption:

`echo 1 | sudo tee /sys/kernel/mm/zns_swap/zns_cgroup_account`

### znGC reclaim watermarks:

For pre-set policies, the low and high watermarks for ZNS GC can be set by:

`echo <wmark_value_in_zones> | sudo tee /sys/kernel/mm/zns_swap/{high,low}_wmark`

## Mount ZNS device as swap:

Create a swap device via:

`sudo mkswap /dev/<zoned_swap_dev>`

Mount the new swap device:

`sudo swapon /dev/<zoned_swap_dev>`

## New statistics:

Procfs file `/proc/vmstat` includes the additional statistics:

```
pzns_gcwrite -> number of page writes performed by znGC
pzns_gcclean -> number of page cache pages released
pzns_gcremap_sc -> number of remapped swap-cache entries
pgznsgcread -> per-cgroup gc reads
pgznsgcwrite -> per-cgroup gc writes
```

New procfs file `/proc/swapstat` includes the additional statistics:

```
nr_total_swapslots -> total number of swap slots available
nr_swapslots -> number of swap slots in use
nr_total_swapclusters -> total number of swap clusters (for traditional block devices)
nr_swapcluster -> number of swap clusters in use
```

## Additional setting for traditonal block swap devices:

* Enable fine-granularity TRIMs: `echo <num_of_pages> | sudo tee /sys/kernel/mm/swap/trim_4k`
* Swap-cache cutoff limit (out of 10): `echo <[0-9]> | sudo tee /sys/kernel/mm/swap/cached_per`

[1]: https://github.com/acsl-technion/znswap_policy_module "ZNSwap example policy module and API description"
