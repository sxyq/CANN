ssh cann-server3 'npu-smi info; printf "\nALL_PROCESSES\n"; ps -eo pid,ppid,user,stat,%cpu,%mem,comm,args --sort=-%cpu'
