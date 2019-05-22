# An-efficient-searching-machine

As part of this work, it was needed to write a program that creates new child processes with the help of the fork () command. Child processes compose a process hierarchy that is in the form of a binary tree. This tree has internal nodes as well as leaf nodes.

The purpose of this project is to perform queries in a 'high-level binary record' and to sort the search results. Processes of the binary process tree implement the search for records in the file. Leaf nodes undertake to search for records in parts of the file while internal nodes compose the results they get from their children and promote them to their parents. The final sorting of the results is done by the sort () system program after the search process has finished.

The Hierarchy of Processes:
The goal of this hierarchy is to help search for records that meet certain criteria from a (large) record. The file is in binary format. The entire binary tree is aimed at a common purpose for which splitters / mergers and searchers work collaboratively. Thus, the nodes perform different executable programs depending on the role they have: the executable splitter / merger and the searcher (s) implement the search, the root makes the orchestration of all the processes and sort sorts the final result. Generally, processes get parameters from their parents to do the job they are assigned to, and they also produce some result (s) and statistics that usually return them to their mother process.

## Compilation
Inside project dictory simply type  ```make
        ```
        
## Running
 Type  ```./myfind –h Height –d Datafile -p Pattern -s
       ```
       
  * myfind is the executable you will create

  * Datafile is the binary data input file

  * Height is the depth of the full binary search tree to be created, with maximum
a setpoint depth = 5. The minimum depth allowed is 1, which means a splitter-merger node and two searchers who search each one in half, depth = 2 means a splitter-merger node with two splitter- each one has two children searcher who can search in one quarter of each file each

  * Pattern is the (sub-) string we look for in the (binary) data file (any field)

  * The display of the -s flag indicates that the searchers nodes are looking for parts of the file that is
unevenly the skew of the records they are looking for

## Deletion of object files and executables
Type  ```make clean
        ```
