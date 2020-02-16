# MPI implementation of MapReduce for creating an inverted index for all the words in text files.

### Example console output:
serban@serban-virtual-machine:\~$ ls  
MapReduce.c  test-files  
serban@serban-virtual-machine:\~$ mpicc MapReduce.c -o MapReduce  
serban@serban-virtual-machine:\~$ mpiexec -np 4 MapReduce test-files output  
[01:00:16] [0-master] Preparing folder output and reading file names  
[01:00:16] [0-master] Done preparing folder output and reading file names  
[01:00:16] [0-master] Calling slaves to count words from files  
[01:08:27] [3-slave] Finished counting words from file test-files/3.txt to folder output/3  
[01:10:05] [3-slave] Finished counting words from file test-files/4.txt to folder output/4  
[01:13:32] [2-slave] Finished counting words from file test-files/1.txt to folder output/1  
[01:16:13] [1-slave] Finished counting words from file test-files/2.txt to folder output/2  
[01:16:25] [3-slave] Finished counting words from file test-files/5.txt to folder output/5  
[01:17:55] [2-slave] Finished counting words from file test-files/6.txt to folder output/6  
[01:20:10] [2-slave] Finished counting words from file test-files/9.txt to folder output/9  
[01:20:30] [3-slave] Finished counting words from file test-files/8.txt to folder output/8  
[01:27:45] [3-slave] Finished counting words from file test-files/11.txt to folder output/11  
[01:32:11] [1-slave] Finished counting words from file test-files/7.txt to folder output/7  
[01:35:05] [3-slave] Finished counting words from file test-files/12.txt to folder output/12  
[01:40:23] [3-slave] Finished counting words from file test-files/14.txt to folder output/14  
[01:40:25] [1-slave] Finished counting words from file test-files/13.txt to folder output/13  
[01:44:04] [2-slave] Finished counting words from file test-files/10.txt to folder output/10  
[01:48:36] [2-slave] Finished counting words from file test-files/17.txt to folder output/17  
[01:51:47] [1-slave] Finished counting words from file test-files/16.txt to folder output/16  
[01:57:30] [3-slave] Finished counting words from file test-files/15.txt to folder output/15  
[02:01:26] [2-slave] Finished counting words from file test-files/18.txt to folder output/18  
[02:03:34] [3-slave] Finished counting words from file test-files/20.txt to folder output/20  
[02:06:37] [2-slave] Finished counting words from file test-files/21.txt to folder output/21  
[02:07:22] [3-slave] Finished counting words from file test-files/22.txt to folder output/22  
[02:10:33] [2-slave] Finished counting words from file test-files/23.txt to folder output/23  
[02:19:14] [3-slave] Finished counting words from file test-files/24.txt to folder output/24  
[02:24:49] [1-slave] Finished counting words from file test-files/19.txt to folder output/19  
[02:31:32] [2-slave] Finished counting words from file test-files/25.txt to folder output/25  
[02:31:32] [0-master] Calling slaves to combine all folders into one  
[02:31:32] [0-master] 25 folders left: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25   
[02:36:00] [2-slave] Finished combining folders output/5 and output/6 into output/p2_f23  
[02:37:19] [3-slave] Finished combining folders output/3 and output/4 into output/p3_f24  
[02:37:45] [1-slave] Finished combining folders output/1 and output/2 into output/p1_f25  
[02:40:39] [2-slave] Finished combining folders output/7 and output/8 into output/p2_f22  
[02:42:11] [1-slave] Finished combining folders output/11 and output/12 into output/p1_f20  
[02:43:23] [3-slave] Finished combining folders output/9 and output/10 into output/p3_f21  
[02:46:10] [2-slave] Finished combining folders output/13 and output/14 into output/p2_f19  
[02:52:17] [3-slave] Finished combining folders output/17 and output/18 into output/p3_f17  
[02:54:01] [1-slave] Finished combining folders output/15 and output/16 into output/p1_f18  
[02:55:19] [3-slave] Finished combining folders output/21 and output/22 into output/p3_f15  
[02:56:34] [1-slave] Finished combining folders output/23 and output/24 into output/p1_f14  
[02:57:22] [2-slave] Finished combining folders output/19 and output/20 into output/p2_f16  
[02:57:22] [0-master] 13 folders left: p1_f25 * p3_f24 * p2_f23 * p2_f22 * p3_f21 * p1_f20 * p2_f19 * p1_f18 * p3_f17 * p2_f16 * p3_f15 * p1_f14 * 25   
[03:06:02] [3-slave] Finished combining folders output/p2_f23 and output/p2_f22 into output/p3_f12  
[03:08:06] [2-slave] Finished combining folders output/p3_f21 and output/p1_f20 into output/p2_f11  
[03:09:34] [1-slave] Finished combining folders output/p1_f25 and output/p3_f24 into output/p1_f13  
[03:14:05] [1-slave] Finished combining folders output/p3_f15 and output/p1_f14 into output/p1_f8  
[03:16:21] [3-slave] Finished combining folders output/p2_f19 and output/p1_f18 into output/p3_f10  
[03:18:04] [2-slave] Finished combining folders output/p3_f17 and output/p2_f16 into output/p2_f9  
[03:18:04] [0-master] 7 folders left: p1_f13 * * * p3_f12 * * * p2_f11 * * * p3_f10 * * * p2_f9 * * * p1_f8 * * * 25   
[03:35:15] [3-slave] Finished combining folders output/p2_f11 and output/p3_f10 into output/p3_f6  
[03:35:19] [1-slave] Finished combining folders output/p1_f13 and output/p3_f12 into output/p1_f7  
[03:35:26] [2-slave] Finished combining folders output/p2_f9 and output/p1_f8 into output/p2_f5  
[03:35:26] [0-master] 4 folders left: p1_f7 * * * * * * * p3_f6 * * * * * * * p2_f5 * * * * * * * 25   
[03:45:34] [3-slave] Finished combining folders output/p2_f5 and output/25 into output/p3_f3  
[03:47:04] [1-slave] Finished combining folders output/p1_f7 and output/p3_f6 into output/p1_f4  
[03:47:04] [0-master] 2 folders left: p1_f4 * * * * * * * * * * * * * * * p3_f3 * * * * * * * *   
[03:59:51] [3-slave] Finished combining folders output/p1_f4 and output/p3_f3 into output/p3_f2  
[03:59:51] [0-master] Writing inverted indexes to file  
[04:01:50] [0-master] Done writing inverted indexes to file  
