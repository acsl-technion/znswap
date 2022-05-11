/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SWAPFILE_H
#define _LINUX_SWAPFILE_H

/*
 * these were static in swapfile.c but frontswap.c needs them and we don't
 * want to expose them to the dozens of source files that include swap.h
 */
extern spinlock_t swap_lock;
extern struct plist_head swap_active_head;
extern struct swap_info_struct *swap_info[];
extern int try_to_unuse(unsigned int, bool, unsigned long);
extern unsigned long generic_max_swapfile_size(void);
extern void reset_swap_zone(struct zns_swap_info_struct *, unsigned int);
extern void wakeup_kznsd (struct zns_swap_info_struct *);
extern void wakeup_kzns_activated (struct zns_swap_info_struct *);
extern unsigned long max_swapfile_size(void);
extern atomic_t swap_outs;
extern atomic_t swap_ins;
extern atomic_t swap_ptes;
extern atomic_t swap_copy;
extern atomic_t swap_unuse;
extern atomic_t swap_zswaps;
extern atomic_t swap_pgouts;
extern atomic_t swap_priv;
extern atomic_t swap_lfree;
extern atomic_t swap_rem_map;

#endif /* _LINUX_SWAPFILE_H */
