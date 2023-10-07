# DPA HW #1 -  Parallel processing using Map-Reduce paradigm

## About the project 

The app is using the **Map-Reduce** paradigm to process a set of input files.
The files will be distributed dynamically to a number of threads given as a
parameter in command line.
Each thread will parse the file and check which numbers are **perfect powers**.
Those numbers will be added in partial lists for each exponent.

*A number is a perfect power if it is a perfect square, perfect cube, etc.* 

The **reducer** process will combine all partial lists from the **mapper** 
process and will build some aggregate lists for each exponent.
Then, unique values will be counted and printed to an output file.


**More**: <a href="/Tema_1_APD.pdf">Project Info [RO]</a>

## Implementation

The app is using 2 types of threads: **mapper** and **reducer**, as the paradigm
suggests.

**Mapper**

- **Args**: 

    - **id**: unique number for each thread
    - **maxExponent**: depending to the number of reducer threads
    - **vizFileNames**: an array that will mark if a file has been processed by
    a thread
    
    - **fileNames**: an array containig input files
    - **partialLists**: data structure that will be created and populated by mapper
    threads.
    It will contain an unordered map on each index. The unordered map
    is using the exponent as a key and a list of those numbers that are perfect
    powers
    - **mutex, barrier**: synchronization elements
    
- **Functionality**
    
  - This type of thread will check in **vizFileNames** array for unallocated files. 
    
  - If there is at least one, it will be parsed by the current thread. 
For each exponent, a partial list will be created by calling **checkPow**
function.

  **checkPow**: iterates through the elements of an array and check if it is a
perfect power. If it does, it will be added to the list that will be returned.
  
  **getNthRoot**: binary search to check if there is a base that raised to the
  given exponent will equal current number (the current number is a perfect
  power)
  
  **multiply**: it will raise a base to an exponent, only if the result is
smaller or equal than *myNumber*. This will optimize the time for larger numbers
and will prevent overflow.

    - Add those numbers that fulfill the property into the list used as a value
    at *exponent* key in the **powMap** unordered map
    - The unordered map is added at the corresponding index into 
    the **partialLists** data structure.
    
    
**Reducer**

- **Args**:

    - **id**: unique number for each thread
    - **exponent**: the exponent that this thread will be responsable of
    - **partialLists**: data structure that will be created and populated by
    mapper threads
    - **barrier**: synchronization elements
    
- **Functionality**

    - The reducers will start only when all mappers have finished their work. 
    This condition will be ensured by the barrier
    - A reducer thread will parse **partialLists** and get the list for the
      corresponding exponent, based on its id.
    - Then, it will add all numbers in a **set** to remove the duplicates.
      
    - The size of the set will be printed to an output file.
    
    
### Built With

* [![C][C]][C-url]

<!-- USAGE EXAMPLES -->
## Usage

To run the app:

1. Use the Makefile to get the `tema1` file
   ```sh
   make
   ```
   
2. Run
```sh
./tema1 NO_MAPPERS NO_REDUCERS INPUT_FILE
```

[C]:https://img.shields.io/badge/C++-00599C?style=flat-square&logo=C%2B%2B&logoColor=white
[C-url]:""