/* internal.h: internal procfs definitions
 *
 * Copyright (C) 2004 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/proc_fs.h>

#ifdef CONFIG_PROC_SYSCTL
extern int proc_sys_init(void);
#else
static inline void proc_sys_init(void) { }
#endif
#ifdef CONFIG_NET
extern int proc_net_init(void);
#else
static inline int proc_net_init(void) { return 0; }
#endif

struct vmalloc_info {
	unsigned long	used;
	unsigned long	largest_chunk;
};

extern struct mm_struct *mm_for_maps(struct task_struct *);

#ifdef CONFIG_MMU
#define VMALLOC_TOTAL (VMALLOC_END - VMALLOC_START)
extern void get_vmalloc_info(struct vmalloc_info *vmi);
#else

#define VMALLOC_TOTAL 0UL
#define get_vmalloc_info(vmi)			\
do {						\
	(vmi)->used = 0;			\
	(vmi)->largest_chunk = 0;		\
} while(0)

extern int nommu_vma_show(struct seq_file *, struct vm_area_struct *);
#endif

extern int maps_protect;

extern void create_seq_entry(char *name, mode_t mode,
				const struct file_operations *f);
extern int proc_exe_link(struct inode *, struct path *);
extern int proc_tid_stat(struct seq_file *m, struct pid_namespace *ns,
				struct pid *pid, struct task_struct *task);
extern int proc_tgid_stat(struct seq_file *m, struct pid_namespace *ns,
				struct pid *pid, struct task_struct *task);
extern int proc_pid_status(struct seq_file *m, struct pid_namespace *ns,
				struct pid *pid, struct task_struct *task);
extern int proc_pid_statm(struct seq_file *m, struct pid_namespace *ns,
				struct pid *pid, struct task_struct *task);
extern loff_t mem_lseek(struct file *file, loff_t offset, int orig);

extern const struct file_operations proc_maps_operations;
extern const struct file_operations proc_numa_maps_operations;
extern const struct file_operations proc_smaps_operations;
extern const struct file_operations proc_clear_refs_operations;
extern const struct file_operations proc_pagemap_operations;
extern const struct file_operations proc_net_operations;
extern const struct inode_operations proc_net_inode_operations;
#ifdef CONFIG_BORPH
extern const struct file_operations proc_bphhw_operations;
extern const struct inode_operations proc_bphhw_inode_operations;
#endif

void free_proc_entry(struct proc_dir_entry *de);

int proc_init_inodecache(void);

static inline struct pid *proc_pid(struct inode *inode)
{
	return PROC_I(inode)->pid;
}

static inline struct task_struct *get_proc_task(struct inode *inode)
{
	return get_pid_task(proc_pid(inode), PIDTYPE_PID);
}

static inline int proc_fd(struct inode *inode)
{
	return PROC_I(inode)->fd;
}

struct dentry *proc_lookup_de(struct proc_dir_entry *de, struct inode *ino,
		struct dentry *dentry);
int proc_readdir_de(struct proc_dir_entry *de, struct file *filp, void *dirent,
		filldir_t filldir);

/*
 * Used in pid entries.  Was defined in base.c. 
 * Moved here for use in proc_borph.c (HS, 07/12/2008)
 */
struct pid_entry {
	char *name;
	int len;
	mode_t mode;
	const struct inode_operations *iop;
	const struct file_operations *fop;
	union proc_op op;
};

#define NOD(NAME, MODE, IOP, FOP, OP) {			\
	.name = (NAME),					\
	.len  = sizeof(NAME) - 1,			\
	.mode = MODE,					\
	.iop  = IOP,					\
	.fop  = FOP,					\
	.op   = OP,					\
}

#define DIR(NAME, MODE, OTYPE)							\
	NOD(NAME, (S_IFDIR|(MODE)),						\
		&proc_##OTYPE##_inode_operations, &proc_##OTYPE##_operations,	\
		{} )
#define LNK(NAME, OTYPE)					\
	NOD(NAME, (S_IFLNK|S_IRWXUGO),				\
		&proc_pid_link_inode_operations, NULL,		\
		{ .proc_get_link = &proc_##OTYPE##_link } )
#define REG(NAME, MODE, OTYPE)				\
	NOD(NAME, (S_IFREG|(MODE)), NULL,		\
		&proc_##OTYPE##_operations, {})
#define INF(NAME, MODE, OTYPE)				\
	NOD(NAME, (S_IFREG|(MODE)), 			\
		NULL, &proc_info_file_operations,	\
		{ .proc_read = &proc_##OTYPE } )
#define ONE(NAME, MODE, OTYPE)				\
	NOD(NAME, (S_IFREG|(MODE)), 			\
		NULL, &proc_single_file_operations,	\
		{ .proc_show = &proc_##OTYPE } )
