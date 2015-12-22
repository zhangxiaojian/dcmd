# dcmd
Excute distributed shell command under MPI environment

How to excute:

  mpiexec -n 7 -f machifile ./dcmd
  while n is the number of nodes in MPI environment.

Usage:
        help,quit,bye
        shell (nodelist)
Example: rm -r ~/csm (123)
The Example remove the directory ~/csm at node 1,2,3. If no node numbser is specified such as "rm -r ~/csm" , then every nodes' direcotry is deleted.
