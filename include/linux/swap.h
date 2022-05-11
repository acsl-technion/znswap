/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SWAP_H
#define _LINUX_SWAP_H

#include <linux/spinlock.h>
#include <linux/linkage.h>
#include <linux/mmzone.h>
#include <linux/list.h>
#include <linux/memcontrol.h>
#include <linux/mempolicy.h>
#include <linux/sched.h>
#include <linux/node.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/page_idle.h>
#include <linux/page-flags.h>
#include <asm/page.h>

struct notifier_block;

struct bio;

struct pagevec;

#define SWAP_FLAG_PREFER	0x8000	/* set if swap priority specified */
#define SWAP_FLAG_PRIO_MASK	0x7fff
#define SWAP_FLAG_PRIO_SHIFT	0
#define SWAP_FLAG_DISCARD	0x10000 /* enable discard for swap */
#define SWAP_FLAG_DISCARD_ONCE	0x20000 /* discard swap area at swapon-time */
#define SWAP_FLAG_DISCARD_PAGES 0x40000 /* discard page-clusters after use */

#define SWAP_FLAGS_VALID	(SWAP_FLAG_PRIO_MASK | SWAP_FLAG_PREFER | \
				 SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE | \
				 SWAP_FLAG_DISCARD_PAGES)
#define SWAP_BATCH 64

#define SWAP_POL_ZONE_SIZE	16UL
#define SWAP_POL_SLOT_SIZE	4UL
#define SWAP_POL_VMA_SIZE	32UL
#define SWAP_POL_SLOT_SHIFT	SWAP_POL_ZONE_SIZE
#define SWAP_POL_VMA_SHIFT	(SWAP_POL_SLOT_SHIFT + SWAP_POL_SLOT_SIZE)
#define SWAP_POL_SEQ_SHIFT	(SWAP_POL_VMA_SHIFT + SWAP_POL_VMA_SIZE)
#define SWAP_POL_EN_SHIFT	(SWAP_POL_VMA_SHIFT + SWAP_POL_VMA_SIZE + 1UL)

#define SWAP_POL_SEQ_FLAG	(1UL << SWAP_POL_SEQ_SHIFT)
#define SWAP_POL_EN_FLAG	(1UL << SWAP_POL_EN_SHIFT)

#define SWAP_POL_ZONE_MASK	((1UL << SWAP_POL_SLOT_SHIFT) - 1)
#define SWAP_POL_SLOT_MASK	(((1UL << SWAP_POL_VMA_SHIFT) - 1) & ~SWAP_POL_ZONE_MASK)
#define SWAP_POL_VMA_MASK	(((1UL << (SWAP_POL_SEQ_SHIFT)) - 1) & \
		~(SWAP_POL_ZONE_MASK | SWAP_POL_SLOT_MASK))


#define SWAP_POL_ZONE(v)	((v) & SWAP_POL_ZONE_MASK)
#define SWAP_POL_SLOT(v)	(((v) & SWAP_POL_SLOT_MASK) >> SWAP_POL_SLOT_SHIFT)
#define SWAP_POL_VMA(v)		(((v) & SWAP_POL_VMA_MASK) >> SWAP_POL_VMA_SHIFT)

#define SWAP_POL_VAL(vma, slot, zone)					\
	 (((1UL) << SWAP_POL_EN_SHIFT) |	\
	 (((vma) << SWAP_POL_VMA_SHIFT) & SWAP_POL_VMA_MASK) |	\
	 (((slot) << SWAP_POL_SLOT_SHIFT) & SWAP_POL_SLOT_MASK) |	\
	 ((zone) & SWAP_POL_ZONE_MASK))

extern void wakeup_kznsd (struct zns_swap_info_struct *);
extern void wakeup_kzns_activated (struct zns_swap_info_struct *);
extern void reset_swap_zone(struct zns_swap_info_struct *, unsigned int);
extern void reset_swap_zone_relaxed(struct zns_swap_info_struct *, unsigned int);
static inline int current_is_kswapd(void)
{
	return current->flags & PF_KSWAPD;
}

#define SWAP_RA_WIN_SHIFT	(PAGE_SHIFT / 2)
#define SWAP_RA_HITS_MASK	((1UL << SWAP_RA_WIN_SHIFT) - 1)
#define SWAP_RA_HITS_MAX	SWAP_RA_HITS_MASK
#define SWAP_RA_WIN_MASK	(~PAGE_MASK & ~SWAP_RA_HITS_MASK)

#define SWAP_RA_HITS(v)		((v) & SWAP_RA_HITS_MASK)
#define SWAP_RA_WIN(v)		(((v) & SWAP_RA_WIN_MASK) >> SWAP_RA_WIN_SHIFT)
#define SWAP_RA_ADDR(v)		((v) & PAGE_MASK)

#define SWAP_RA_VAL(addr, win, hits)				\
	(((addr) & PAGE_MASK) |					\
	 (((win) << SWAP_RA_WIN_SHIFT) & SWAP_RA_WIN_MASK) |	\
	 ((hits) & SWAP_RA_HITS_MASK))

/* Initial readahead hits is 4 to start up with a small window */
#define GET_SWAP_RA_VAL(vma)					\
	(atomic_long_read(&(vma)->swap_readahead_info) ? : 4)

/*
 * MAX_SWAPFILES defines the maximum number of swaptypes: things which can
 * be swapped to.  The swap type and the offset into that swap type are
 * encoded into pte's and into pgoff_t's in the swapcache.  Using five bits
 * for the type means that the maximum number of swapcache pages is 27 bits
 * on 32-bit-pgoff_t architectures.  And that assumes that the architecture packs
 * the type/offset into the pte as 5/27 as well.
 */
#define MAX_SWAPFILES_SHIFT	5

/*
 * Use some of the swap files numbers for other purposes. This
 * is a convenient way to hook into the VM to trigger special
 * actions on faults.
 */

/*
 * Unaddressable device memory support. See include/linux/hmm.h and
 * Documentation/vm/hmm.rst. Short description is we need struct pages for
 * device memory that is unaddressable (inaccessible) by CPU, so that we can
 * migrate part of a process memory to device memory.
 *
 * When a page is migrated from CPU to device, we set the CPU page table entry
 * to a special SWP_DEVICE_* entry.
 */
#ifdef CONFIG_DEVICE_PRIVATE
#define SWP_DEVICE_NUM 2
#define SWP_DEVICE_WRITE (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM)
#define SWP_DEVICE_READ (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM+1)
#else
#define SWP_DEVICE_NUM 0
#endif

/*
 * NUMA node memory migration support
 */
#ifdef CONFIG_MIGRATION
#define SWP_MIGRATION_NUM 2
#define SWP_MIGRATION_READ	(MAX_SWAPFILES + SWP_HWPOISON_NUM)
#define SWP_MIGRATION_WRITE	(MAX_SWAPFILES + SWP_HWPOISON_NUM + 1)
#else
#define SWP_MIGRATION_NUM 0
#endif

/*
 * Handling of hardware poisoned pages with memory corruption.
 */
#ifdef CONFIG_MEMORY_FAILURE
#define SWP_HWPOISON_NUM 1
#define SWP_HWPOISON		MAX_SWAPFILES
#else
#define SWP_HWPOISON_NUM 0
#endif

#define MAX_SWAPFILES \
	((1 << MAX_SWAPFILES_SHIFT) - SWP_DEVICE_NUM - \
	SWP_MIGRATION_NUM - SWP_HWPOISON_NUM)

/*
 * Magic header for a swap area. The first part of the union is
 * what the swap magic looks like for the old (limited to 128MB)
 * swap area format, the second part of the union adds - in the
 * old reserved area - some extra information. Note that the first
 * kilobyte is reserved for boot loader or disk label stuff...
 *
 * Having the magic at the end of the PAGE_SIZE makes detecting swap
 * areas somewhat tricky on machines that support multiple page sizes.
 * For 2.5 we'll probably want to move the magic to just beyond the
 * bootbits...
 */
union swap_header {
	struct {
		char reserved[PAGE_SIZE - 10];
		char magic[10];			/* SWAP-SPACE or SWAPSPACE2 */
	} magic;
	struct {
		char		bootbits[1024];	/* Space for disklabel etc. */
		__u32		version;
		__u32		last_page;
		__u32		nr_badpages;
		unsigned char	sws_uuid[16];
		unsigned char	sws_volume[16];
		__u32		padding[117];
		__u32		badpages[1];
	} info;
};

/*
 * current->reclaim_state points to one of these when a task is running
 * memory reclaim
 */
struct reclaim_state {
	unsigned long reclaimed_slab;
};

#ifdef __KERNEL__

struct address_space;
struct sysinfo;
struct writeback_control;
struct zone;

/*
 * A swap extent maps a range of a swapfile's PAGE_SIZE pages onto a range of
 * disk blocks.  A list of swap extents maps the entire swapfile.  (Where the
 * term `swapfile' refers to either a blockdevice or an IS_REG file.  Apart
 * from setup, they're handled identically.
 *
 * We always assume that blocks are of size PAGE_SIZE.
 */
struct swap_extent {
	struct rb_node rb_node;
	pgoff_t start_page;
	pgoff_t nr_pages;
	sector_t start_block;
};

/*
 * Max bad pages in the new format..
 */
#define MAX_SWAP_BADPAGES \
	((offsetof(union swap_header, magic.magic) - \
	  offsetof(union swap_header, info.badpages)) / sizeof(int))

enum {
	SWP_USED	= (1 << 0),	/* is slot in swap_info[] used? */
	SWP_WRITEOK	= (1 << 1),	/* ok to write to this swap?	*/
	SWP_DISCARDABLE = (1 << 2),	/* blkdev support discard */
	SWP_DISCARDING	= (1 << 3),	/* now discarding a free cluster */
	SWP_SOLIDSTATE	= (1 << 4),	/* blkdev seeks are cheap */
	SWP_CONTINUED	= (1 << 5),	/* swap_map has count continuation */
	SWP_BLKDEV	= (1 << 6),	/* its a block device */
	SWP_ACTIVATED	= (1 << 7),	/* set after swap_activate success */
	SWP_FS_OPS	= (1 << 8),	/* swapfile operations go through fs */
	SWP_AREA_DISCARD = (1 << 9),	/* single-time swap area discards */
	SWP_PAGE_DISCARD = (1 << 10),	/* freed swap page-cluster discards */
	SWP_STABLE_WRITES = (1 << 11),	/* no overwrite PG_writeback pages */
	SWP_SYNCHRONOUS_IO = (1 << 12),	/* synchronous IO is efficient */
	SWP_VALID	= (1 << 13),	/* swap is valid to be operated on? */
					/* add others here before... */
	SWP_SCANNING	= (1 << 14),	/* refcount in scan_swap_map */
};

#define SWAP_CLUSTER_MAX 32UL
#define COMPACT_CLUSTER_MAX SWAP_CLUSTER_MAX

/* Bit flag in swap_map */
#define SWAP_HAS_CACHE	0x40	/* Flag page is cached, in first swap_map */
#define COUNT_CONTINUED	0x80	/* Flag swap_map continuation for full count */

/* Special value in first swap_map */
#define SWAP_MAP_MAX	0x3e	/* Max count */
#define SWAP_MAP_BAD	0x3f	/* Note page is bad */
#define SWAP_MAP_SHMEM	0xbf	/* Owned by shmem/tmpfs */

/* Special value in each swap_map continuation */
#define SWAP_CONT_MAX	0x7f	/* Max count */

/*
 * We use this to track usage of a cluster. A cluster is a block of swap disk
 * space with SWAPFILE_CLUSTER pages long and naturally aligns in disk. All
 * free clusters are organized into a list. We fetch an entry from the list to
 * get a free cluster.
 *
 * The data field stores next cluster if the cluster is free or cluster usage
 * counter otherwise. The flags field determines if a cluster is free. This is
 * protected by swap_info_struct.lock.
 */
struct swap_cluster_info {
	spinlock_t lock;	/*
				 * Protect swap_cluster_info fields
				 * and swap_info_struct->swap_map
				 * elements correspond to the swap
				 * cluster
				 */
	unsigned int data:24;
	unsigned int flags:8;
};
#define CLUSTER_FLAG_FREE 1 /* This cluster is free */
#define CLUSTER_FLAG_NEXT_NULL 2 /* This cluster has no next cluster */
#define CLUSTER_FLAG_HUGE 4 /* This cluster is backing a transparent huge page */

/*
 * We assign a cluster to each CPU, so each CPU can allocate swap entry from
 * its own cluster and swapout sequentially. The purpose is to optimize swapout
 * throughput.
 */
struct percpu_cluster {
	struct swap_cluster_info index; /* Current cluster index */
	unsigned int next; /* Likely next allocation offset */
};

struct swap_cluster_list {
	struct swap_cluster_info head;
	struct swap_cluster_info tail;
};

/* 4MiB buffer */
#define ZNS_GC_ORDER 10 /* page order */
#define ZNS_GC_BYTES (1UL << (ZNS_GC_ORDER + PAGE_SHIFT))
#define ZNS_GC_PAGES (ZNS_GC_BYTES >> 12)

extern bool monitor_write_ratio;
extern bool zns_en;
/* In-memory structure for zns swap areas */
enum zns_flags {
	ZNS_SWAP_UNDER_GC = 0,
	ZNS_SWAP_SUSPEND,
};

enum status {
	ZNS_GC_IDLE,
	ZNS_GC_READING,
	ZNS_GC_WRITING,
};

enum alloc_policy {
	ZNS_POLICY_NAIVE = 0,
	ZNS_POLICY_CPU,
	ZNS_POLICY_THREAD,
	ZNS_POLICY_RM_VMA,
	ZNS_POLICY_STATIC_HEAT,
	ZNS_POLICY_CGROUP,
	ZNS_POLICY_MODULE,
};

struct reclaim_ctx {
	struct page *buffer;
	atomic_t finished_read;
	atomic_t finished_write;
	unsigned int num_pages;
	unsigned int last_pos;
	struct bio *move_bios[ZNS_GC_PAGES];
	enum status stat;
	int from_zone;
};

struct swap_zone {
	atomic_t count;		/* Number of available slots in a zone */
	atomic_t slot_count;
	atomic_t invalid;		/* Number of invalid slots in a zone */
	atomic_t has_cache;		/* Number of invalid slots in a zone */
	unsigned char *swap_map;	/* vmalloc'ed array of usage counts */
	spinlock_t *slot_lock;
	struct bio reset_bio;
	atomic_t open;			/* 1- taken, 2- open for write,
					   3- finished			*/
	int cur_open_slot;
};

struct fine_discard_work {
	struct work_struct work;
	unsigned long page_nr;
	struct swap_info_struct *p;
};

struct zns_swap_info_struct {
	unsigned int num_zones;		/* Total num zones on swap device */
	atomic_t available_open_zones;
	unsigned int max_open_zones;	/* maximum number of open zones, minus
					   one for GC			  */
	atomic_t *open_zones;	/* max open zones -1 */
	atomic_t *using_open_zones;	/* max open zones -1 */
	atomic_t last_alloced_zone;

	enum alloc_policy zns_policy;
	int (*policy_func)(u64, struct swappolicy*);
	bool zns_cgroup_account;
	struct mem_cgroup *memcg;
	unsigned int zone_size;		/* Number of page slots in a zone */
	unsigned int zone_capacity;		/* Number of page slots in a zone */
	atomic_long_t inuse_pages;
	struct swap_zone *swap_zones;
	struct swap_info_struct *si;	/* May not be necessary if i can get
					   container of working */
	unsigned int low_wmark;
	unsigned int high_wmark;
	struct reclaim_ctx rctx;
	wait_queue_head_t gc_wait;
	struct task_struct *gc;
	unsigned long *gc_waiting_bitmap;
	unsigned long *module_waiting_bitmap;
	atomic_t emergency_gc_zone;
	atomic_t gc_in_use;
	atomic_t free_zones;
	unsigned long flags;
	unsigned int start_bucket_order;
};

/* API structs */
struct pg_i {
	u64 last_swapout_t;
	u16 access_bit_vec;
	int owner_pid;
	u64 cgroup_id;
};

struct zn_i {
	int zone_id;
	int capacity;
	int occupied_slots;
	int invalid_slots;
	int swap_cache_slots;
	int swap_zone_id;
};

struct vm_i {
	u64 vm_flags;
	u64 size;
	int readahead_win_sz;
	u64 cgroup_id;
};

struct swap_i {
	u64 num_slots;
	u64 num_zns;
	u64 free_slots;
	u64 free_zns;
	u8 zslot_array_sz;
	u32 high_wmark;
	u32 low_wmark;
	bool gc_running;
};

extern void rec_zn(int zn);
extern void pg_inf(struct pg_i* pg, u64 pfn, struct swappolicy *sp);
extern void vm_inf(struct vm_i* vm, u64 pfn, struct swappolicy *sp);
extern void zn_inf(struct zn_i* zn, int zone);
extern void swap_inf(struct swap_i* swap);
extern void register_policy(int(*pol)(u64, struct swappolicy*));
/* Finish API structs */

/*
 * The in-memory structure used to track swap areas.
 */
struct swap_info_struct {
	unsigned long	flags;		/* SWP_USED etc: see above */
	signed short	prio;		/* swap priority of this type */
	struct plist_node list;		/* entry in swap_active_head */
	signed char	type;		/* strange name for an index */
	unsigned int	max;		/* extent of the swap_map */
	unsigned char *swap_map;	/* vmalloc'ed array of usage counts */
	struct swap_cluster_info *cluster_info; /* cluster info. Only for SSD */
	struct swap_cluster_list free_clusters; /* free clusters list */
	unsigned int lowest_bit;	/* index of first free in swap_map */
	unsigned int highest_bit;	/* index of last free in swap_map */
	unsigned int pages;		/* total of usable pages of swap */
	unsigned int inuse_pages;	/* number of those currently in use */
	unsigned int cluster_next;	/* likely index for next allocation */
	unsigned int cluster_nr;	/* countdown to next cluster search */
	unsigned int __percpu *cluster_next_cpu; /*percpu index for next allocation */
	struct percpu_cluster __percpu *percpu_cluster; /* per cpu's swap location */
	struct rb_root swap_extent_root;/* root of the swap extent rbtree */
	struct block_device *bdev;	/* swap device or bdev of swap file */
	struct file *swap_file;		/* seldom referenced */
	unsigned int old_block_size;	/* seldom referenced */
#ifdef CONFIG_FRONTSWAP
	unsigned long *frontswap_map;	/* frontswap in-use, one bit per page */
	atomic_t frontswap_pages;	/* frontswap pages in-use counter */
#endif
	struct workqueue_struct *discard_workers;
	spinlock_t lock;		/*
					 * protect map scan related fields like
					 * swap_map, lowest_bit, highest_bit,
					 * inuse_pages, cluster_next,
					 * cluster_nr, lowest_alloc,
					 * highest_alloc, free/discard cluster
					 * list. other fields are only changed
					 * at swapon/swapoff, so are protected
					 * by swap_lock. changing flags need
					 * hold this lock and swap_lock. If
					 * both locks need hold, hold swap_lock
					 * first.
					 */
	spinlock_t cont_lock;		/*
					 * protect swap count continuation page
					 * list.
					 */
	struct work_struct discard_work; /* discard worker */
	struct swap_cluster_list discard_clusters; /* discard clusters list */
	struct zns_swap_info_struct *zns_swap; /* ZNS specific swap info */
	struct plist_node avail_lists[]; /*
					   * entries in swap_avail_heads, one
					   * entry per node.
					   * Must be last as the number of the
					   * array is nr_node_ids, which is not
					   * a fixed value so have to allocate
					   * dynamically.
					   * And it has to be an array so that
					   * plist_for_each_* can work.
					   */
};
extern struct swap_info_struct *zns_si;
extern struct swap_zone *zns_sz;

#ifdef CONFIG_64BIT
#define SWAP_RA_ORDER_CEILING	5
#else
/* Avoid stack overflow, because we need to save part of page table */
#define SWAP_RA_ORDER_CEILING	3
#define SWAP_RA_PTE_CACHE_SIZE	(1 << SWAP_RA_ORDER_CEILING)
#endif

struct vma_swap_readahead {
	unsigned short win;
	unsigned short offset;
	unsigned short nr_pte;
#ifdef CONFIG_64BIT
	pte_t *ptes;
#else
	pte_t ptes[SWAP_RA_PTE_CACHE_SIZE];
#endif
};

/* linux/mm/workingset.c */
void workingset_age_nonresident(struct lruvec *lruvec, unsigned long nr_pages);
void *workingset_eviction(struct page *page, struct mem_cgroup *target_memcg);
void workingset_refault(struct page *page, void *shadow);
void workingset_activation(struct page *page);

/* Only track the nodes of mappings with shadow entries */
void workingset_update_node(struct xa_node *node);
#define mapping_set_update(xas, mapping) do {				\
	if (!dax_mapping(mapping) && !shmem_mapping(mapping))		\
		xas_set_update(xas, workingset_update_node);		\
} while (0)

/* linux/mm/page_alloc.c */
extern unsigned long totalreserve_pages;
extern unsigned long nr_free_buffer_pages(void);

/* Definition of global_zone_page_state not available yet */
#define nr_free_pages() global_zone_page_state(NR_FREE_PAGES)


/* linux/mm/swap.c */
extern void lru_note_cost(struct lruvec *lruvec, bool file,
			  unsigned int nr_pages);
extern void lru_note_cost_page(struct page *);
extern void lru_cache_add(struct page *);
extern void mark_page_accessed(struct page *);
extern void lru_add_drain(void);
extern void lru_add_drain_cpu(int cpu);
extern void lru_add_drain_cpu_zone(struct zone *zone);
extern void lru_add_drain_all(void);
extern void rotate_reclaimable_page(struct page *page);
extern void deactivate_file_page(struct page *page);
extern void deactivate_page(struct page *page);
extern void mark_page_lazyfree(struct page *page);
extern void swap_setup(void);

extern void lru_cache_add_inactive_or_unevictable(struct page *page,
						struct vm_area_struct *vma);

/* linux/mm/vmscan.c */
extern unsigned long zone_reclaimable_pages(struct zone *zone);
extern unsigned long try_to_free_pages(struct zonelist *zonelist, int order,
					gfp_t gfp_mask, nodemask_t *mask);
extern bool __isolate_lru_page_prepare(struct page *page, isolate_mode_t mode);
extern unsigned long try_to_free_mem_cgroup_pages(struct mem_cgroup *memcg,
						  unsigned long nr_pages,
						  gfp_t gfp_mask,
						  bool may_swap);
extern unsigned long mem_cgroup_shrink_node(struct mem_cgroup *mem,
						gfp_t gfp_mask, bool noswap,
						pg_data_t *pgdat,
						unsigned long *nr_scanned);
extern unsigned long shrink_all_memory(unsigned long nr_pages);
extern int vm_swappiness;
extern int remove_mapping(struct address_space *mapping, struct page *page);

extern unsigned long reclaim_pages(struct list_head *page_list);
#ifdef CONFIG_NUMA
extern int node_reclaim_mode;
extern int sysctl_min_unmapped_ratio;
extern int sysctl_min_slab_ratio;
#else
#define node_reclaim_mode 0
#endif

extern void check_move_unevictable_pages(struct pagevec *pvec);

extern int kswapd_run(int nid);
extern void kswapd_stop(int nid);

extern int kzns_activated_run(struct zns_swap_info_struct *zi, int nid);
extern void kzns_activated_stop(struct zns_swap_info_struct *zi);

extern int kznsd_run(struct zns_swap_info_struct *zi, int nid);
extern void kznsd_stop(struct zns_swap_info_struct *zi);

#ifdef CONFIG_SWAP

#include <linux/blk_types.h> /* for bio_end_io_t */

/* linux/mm/page_io.c */
extern int swap_readpage(struct page *page, bool do_poll);
extern int swap_writepage(struct page *page, struct writeback_control *wbc);
extern void end_swap_bio_write(struct bio *bio);
extern int __swap_writepage(struct page *page, struct writeback_control *wbc,
	bio_end_io_t end_write_func);
extern int swap_set_page_dirty(struct page *page);

int add_swap_extent(struct swap_info_struct *sis, unsigned long start_page,
		unsigned long nr_pages, sector_t start_block);
int generic_swapfile_activate(struct swap_info_struct *, struct file *,
		sector_t *);

/* linux/mm/swap_state.c */
/* One swap address space for each 64M swap space */
#define SWAP_ADDRESS_SPACE_SHIFT	14
#define SWAP_ADDRESS_SPACE_PAGES	(1 << SWAP_ADDRESS_SPACE_SHIFT)
extern struct address_space *swapper_spaces[];
#define swap_address_space(entry)			    \
	(&swapper_spaces[swp_type(entry)][swp_offset(entry) \
		>> SWAP_ADDRESS_SPACE_SHIFT])
static inline unsigned long total_swapcache_pages(void)
{
	return global_node_page_state(NR_SWAPCACHE);
}

extern unsigned long total_swapcache_pages(void);
extern void show_swap_cache_info(void);
extern int add_to_zswap(struct page *page);
extern int add_to_swap(struct page *page, struct swappolicy *sp,
		bool *requires_flush);
extern void *get_shadow_from_swap_cache(swp_entry_t entry);
extern inline int add_to_swap_cache(struct page *page, swp_entry_t entry,
			gfp_t gfp, void **shadowp);
extern int _add_to_swap_cache(struct page *page, swp_entry_t entry,
			gfp_t gfp, void **shadowp, bool);
extern void ___delete_from_swap_cache(struct page *page,
			swp_entry_t entry, void *shadow, bool);
inline extern void __delete_from_swap_cache(struct page *page,
			swp_entry_t entry, void *shadow);
extern void delete_from_swap_cache(struct page *);
extern void clear_shadow_from_swap_cache(int type, unsigned long begin,
				unsigned long end);
extern void free_page_and_swap_cache(struct page *);
extern void free_pages_and_swap_cache(struct page **, int);
extern struct page *lookup_swap_cache(swp_entry_t entry,
				      struct vm_area_struct *vma,
				      unsigned long addr);
struct page *find_get_incore_page(struct address_space *mapping, pgoff_t index);
extern struct page *read_swap_cache_async(swp_entry_t, gfp_t,
			struct vm_area_struct *vma, unsigned long addr,
			bool do_poll);
extern struct page *__read_swap_cache_async(swp_entry_t, gfp_t,
			struct vm_area_struct *vma, unsigned long addr,
			bool *new_page_allocated);
extern struct page *swap_cluster_readahead(swp_entry_t entry, gfp_t flag,
				struct vm_fault *vmf);
extern struct page *swapin_readahead(swp_entry_t entry, gfp_t flag,
				struct vm_fault *vmf);

/* linux/mm/swapfile.c */
extern atomic_long_t nr_swap_pages;
extern atomic_long_t nr_swap_clusters;
extern long total_swap_pages;
extern long total_swap_clusters;
extern atomic_t nr_rotate_swap;
extern bool has_usable_swap(void);
extern int cached_per;
extern int trim_4k;

/* Yes, I know, this is fugly, but this is a POC after all (at least not POS) */
static inline bool zns_enabled(void)
{
	return zns_en;
}
/* Swap 50% full? Release swapcache more aggressively.. */
static inline bool vm_swap_full(void)
{
	if (zns_enabled())
		return false;

	return (atomic_long_read(&nr_swap_pages) * 10 < total_swap_pages * (10 - cached_per));
}

static inline long get_nr_swap_pages(void)
{
	return atomic_long_read(&nr_swap_pages);
}

extern void si_swapinfo(struct sysinfo *);
extern swp_entry_t get_swap_page(struct page *page, struct swappolicy *sp,
		bool *requires_flush);
extern void put_swap_page(struct page *page, swp_entry_t entry);
extern swp_entry_t get_swap_page_of_type(int);
extern int get_swap_pages(int n, swp_entry_t swp_entries[], int entry_size);
extern int get_zns_swap_page(struct swap_info_struct *si, swp_entry_t swp_entries[],
		struct page *, struct swappolicy *, bool *);
extern int add_swap_count_continuation(swp_entry_t, gfp_t);
extern void swap_shmem_alloc(swp_entry_t);
extern int swap_duplicate(swp_entry_t);
extern int swap_duplicate_for_new(swp_entry_t);
extern int swapcache_prepare(swp_entry_t);
extern void swap_free(swp_entry_t);
extern void swapcache_free_entries(swp_entry_t *entries, int n);
extern int free_swap_and_cache(swp_entry_t);
int swap_type_of(dev_t device, sector_t offset);
int find_first_swap(dev_t *device);
extern unsigned int count_swap_pages(int, int);
extern sector_t swapdev_block(int, pgoff_t);
extern int page_swapcount(struct page *);
extern int __swap_count(swp_entry_t entry);
extern int __swp_swapcount(swp_entry_t entry);
extern int swp_swapcount(swp_entry_t entry);
extern struct swap_info_struct *page_swap_info(struct page *);
extern struct swap_info_struct *swp_swap_info(swp_entry_t entry);
extern bool reuse_swap_page(struct page *, int *);
extern int try_to_free_swap(struct page *);
struct backing_dev_info;
extern int init_swap_address_space(unsigned int type, unsigned long nr_pages);
extern void exit_swap_address_space(unsigned int type);
extern struct swap_info_struct *get_swap_device(swp_entry_t entry);
extern void _swap_entry_free(struct swap_info_struct *p, swp_entry_t entry, bool);
sector_t swap_page_sector(struct page *page);

static inline void put_swap_device(struct swap_info_struct *si)
{
	rcu_read_unlock();
}

static inline void check_zone_w_inv(struct zns_swap_info_struct *zi, int zone, int invalids)
{
	int cap = zi->zone_capacity;

	cap -= invalids;
	if (!cap) {
		reset_swap_zone(zi, zone);
		return;
	}
}

static inline struct swap_info_struct *get_zns_si(void)
{
	return zns_si;
}

static inline bool do_monitor_write_ratio(void)
{
	return false;
}

#else /* CONFIG_SWAP */

static inline int swap_readpage(struct page *page, bool do_poll)
{
	return 0;
}

static inline struct swap_info_struct *swp_swap_info(swp_entry_t entry)
{
	return NULL;
}

#define swap_address_space(entry)		(NULL)
#define get_nr_swap_pages()			0L
#define total_swap_pages			0L
#define total_swapcache_pages()			0UL
#define vm_swap_full()				0

#define si_swapinfo(val) \
	do { (val)->freeswap = (val)->totalswap = 0; } while (0)
/* only sparc can not include linux/pagemap.h in this file
 * so leave put_page and release_pages undeclared... */
#define free_page_and_swap_cache(page) \
	put_page(page)
#define free_pages_and_swap_cache(pages, nr) \
	release_pages((pages), (nr));

static inline void show_swap_cache_info(void)
{
}

#define free_swap_and_cache(e) ({(is_migration_entry(e) || is_device_private_entry(e));})
#define swapcache_prepare(e) ({(is_migration_entry(e) || is_device_private_entry(e));})

static inline int add_swap_count_continuation(swp_entry_t swp, gfp_t gfp_mask)
{
	return 0;
}

static inline void swap_shmem_alloc(swp_entry_t swp)
{
}

static inline int swap_duplicate(swp_entry_t swp)
{
	return 0;
}

static inline void swap_free(swp_entry_t swp)
{
}

static inline void put_swap_page(struct page *page, swp_entry_t swp)
{
}

static inline struct page *swap_cluster_readahead(swp_entry_t entry,
				gfp_t gfp_mask, struct vm_fault *vmf)
{
	return NULL;
}

static inline struct page *swapin_readahead(swp_entry_t swp, gfp_t gfp_mask,
			struct vm_fault *vmf)
{
	return NULL;
}

static inline int swap_writepage(struct page *p, struct writeback_control *wbc)
{
	return 0;
}

static inline struct page *lookup_swap_cache(swp_entry_t swp,
					     struct vm_area_struct *vma,
					     unsigned long addr)
{
	return NULL;
}

static inline
struct page *find_get_incore_page(struct address_space *mapping, pgoff_t index)
{
	return find_get_page(mapping, index);
}

static inline int add_to_swap(struct page *page)
{
	return 0;
}

static inline void *get_shadow_from_swap_cache(swp_entry_t entry)
{
	return NULL;
}

static inline int add_to_swap_cache(struct page *page, swp_entry_t entry,
					gfp_t gfp_mask, void **shadowp)
{
	return -1;
}

static inline void __delete_from_swap_cache(struct page *page,
					swp_entry_t entry, void *shadow)
{
}

static inline void delete_from_swap_cache(struct page *page)
{
}

static inline void clear_shadow_from_swap_cache(int type, unsigned long begin,
				unsigned long end)
{
}

static inline int page_swapcount(struct page *page)
{
	return 0;
}

static inline int __swap_count(swp_entry_t entry)
{
	return 0;
}

static inline int __swp_swapcount(swp_entry_t entry)
{
	return 0;
}

static inline int swp_swapcount(swp_entry_t entry)
{
	return 0;
}

#define reuse_swap_page(page, total_map_swapcount) \
	(page_trans_huge_mapcount(page, total_map_swapcount) == 1)

static inline int try_to_free_swap(struct page *page)
{
	return 0;
}

static inline swp_entry_t get_swap_page(struct page *page)
{
	swp_entry_t entry;
	entry.val = 0;
	return entry;
}

#endif /* CONFIG_SWAP */

#ifdef CONFIG_THP_SWAP
extern int split_swap_cluster(swp_entry_t entry);
#else
static inline int split_swap_cluster(swp_entry_t entry)
{
	return 0;
}
#endif

#ifdef CONFIG_MEMCG
static inline int mem_cgroup_swappiness(struct mem_cgroup *memcg)
{
	/* Cgroup2 doesn't have per-cgroup swappiness */
	if (cgroup_subsys_on_dfl(memory_cgrp_subsys))
		return vm_swappiness;

	/* root ? */
	if (mem_cgroup_disabled() || mem_cgroup_is_root(memcg))
		return vm_swappiness;

	return memcg->swappiness;
}
#else
static inline int mem_cgroup_swappiness(struct mem_cgroup *mem)
{
	return vm_swappiness;
}
#endif

#if defined(CONFIG_SWAP) && defined(CONFIG_MEMCG) && defined(CONFIG_BLK_CGROUP)
extern void cgroup_throttle_swaprate(struct page *page, gfp_t gfp_mask);
#else
static inline void cgroup_throttle_swaprate(struct page *page, gfp_t gfp_mask)
{
}
#endif

#ifdef CONFIG_MEMCG_SWAP
extern void mem_cgroup_swapout(struct page *page, swp_entry_t entry);
extern int mem_cgroup_try_charge_swap(struct page *page, swp_entry_t entry);
extern void mem_cgroup_uncharge_swap(swp_entry_t entry, unsigned int nr_pages);
extern long mem_cgroup_get_nr_swap_pages(struct mem_cgroup *memcg);
extern bool mem_cgroup_swap_full(struct page *page);
#else
static inline void mem_cgroup_swapout(struct page *page, swp_entry_t entry)
{
}

static inline int mem_cgroup_try_charge_swap(struct page *page,
					     swp_entry_t entry)
{
	return 0;
}

static inline void mem_cgroup_uncharge_swap(swp_entry_t entry,
					    unsigned int nr_pages)
{
}

static inline long mem_cgroup_get_nr_swap_pages(struct mem_cgroup *memcg)
{
	return get_nr_swap_pages();
}

static inline bool mem_cgroup_swap_full(struct page *page)
{
	return vm_swap_full();
}
#endif

#endif /* __KERNEL__*/
#endif /* _LINUX_SWAP_H */
