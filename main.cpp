#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <math.h>
#include <pthread.h>
#include <set>
#include <stdio.h>
#include <unordered_map>
#include <vector>
using namespace std;

#define MAX_NO_FILES 5000

typedef struct
{
    int id;
    int maxExponent;
    int *vizFileNames;
    vector<string> *fileNames;
    vector<unordered_map<int, list<int>>> *partialLists;
    pthread_mutex_t *mutex;
    pthread_barrier_t *barrier;
} mapperArgs_T;

typedef struct
{
    int id;
    int exponent;
    vector<unordered_map<int, list<int>>> *partialLists;
    pthread_barrier_t *barrier;
} reducerArgs_T;

// Calculate pow(number, n)
long long multiply(int number, int n, int myNumber)
{
    long ans = 1;
    for (int i = 1; i <= n; i++)
    {
        // Stop if the result is already greater than current number
        if (ans > myNumber)
        {
            return myNumber + 1;
        }
        ans = (ans * number);
    }
    return ans;
}

bool getNthRoot(int n, int m)
{
    // Bounds optimization, maximum mid = sqrt(m)
    int low = 1;
    int high = 2 * sqrt(m);

    // Binary search for our nth root
    while (low <= high)
    {
        int mid = (low + high) / 2;
        long mult = multiply(mid, n, m);
        if (mult == m)
        {
            return true;
        }
        else if (mult < m)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return false;
}

list<int> checkPow(vector<int> numbers, int exponent)
{
    list<int> baseLists;

    for (long unsigned int i = 0; i < numbers.size(); i++)
    {
        int number = numbers[i];

        // If the number is equal to 1, then is a perfect power for any exponent
        if (number == 1)
        {
            baseLists.push_back(number);
            continue;
        }

        // If the number is a perfect power, add it to the list
        if (getNthRoot(exponent, number))
        {
            baseLists.push_back(number);
        }
    }

    return baseLists;
}

// Function to print a list
void showlist(list<int> g)
{
    cout << "{";
    list<int>::iterator it;
    for (it = g.begin(); it != g.end(); ++it)
        cout << *it << " ";
    cout << "}";
    cout << '\n';
}

// Print a map built by a mapper
void printMap(unordered_map<int, list<int>> powMap)

{
    for (auto it = powMap.begin(); it != powMap.end(); ++it)
    {
        cout << "{" << it->first << " , ";
        showlist(it->second);
        cout << "}";
        cout << '\n';
    }
}

// Mapper function
void *mapper(void *arg)
{
    mapperArgs_T *mapper = (mapperArgs_T *)arg;

    unordered_map<int, list<int>> powMap;
    string fileName;
    int currentFile = -1;

    while (currentFile == -1)
    {
        pthread_mutex_lock(mapper->mutex);

        // Check if there are some unallocated files
        for (long unsigned int i = 0; i < mapper->fileNames->size(); i++)
        {
            if (mapper->vizFileNames[i] == 0)
            {
                // Mark it as allocated
                mapper->vizFileNames[i] = 1;
                currentFile = i;
                break;
            }
        }

        pthread_mutex_unlock(mapper->mutex);

        // If no unallocated files were found
        if (currentFile == -1)
        {
            break;
        }

        // Assign the file to the current mapper thread
        fileName = mapper->fileNames->at(currentFile);

        // Choose file by file name
        ifstream fin;
        fin.open(fileName);
        if (fin.fail())
        {
            cout << "Eroare la deschidrea fisierului";
            exit(-1);
        }

        // Create numbers array
        int n;
        fin >> n;

        vector<int> numbers;
        while (n > 0)
        {
            int x;
            fin >> x;
            numbers.push_back(x);
            n--;
        }

        // Get a list for each exponent
        for (int i = 2; i <= mapper->maxExponent; i++)
        {
            // Create a list and populate it with numbers that
            // are perfect pwers for the current exponent
            list<int> exponentList(checkPow(numbers, i));

            // Check if the list already exist
            if (powMap.count(i) == 0)
            {
                powMap[i] = exponentList;
            }

            else
            {
                for (auto it = exponentList.begin(); it != exponentList.end(); it++)
                {
                    powMap.at(i).push_back(*it);
                }
            }
        }

        fin.close();
        currentFile = -1;
    }

    // Store the map for the current mapper
    pthread_mutex_lock(mapper->mutex);
    mapper->partialLists->at(mapper->id) = powMap;
    pthread_mutex_unlock(mapper->mutex);
    pthread_barrier_wait(mapper->barrier);
    pthread_exit(NULL);
}

void *reducer(void *arg)
{
    reducerArgs_T *reducer = (reducerArgs_T *)arg;
    pthread_barrier_wait(reducer->barrier);
    set<int> reducerSet;

    for (int i = 0; i < reducer->partialLists->size(); i++)
    {
        // Get the mapper list for the current exponent (= reducer_id + 2)
        auto mapperIt = reducer->partialLists->at(i).find(reducer->exponent);
        if (mapperIt != reducer->partialLists->at(i).end())
        {
            for (list<int>::iterator listIt = mapperIt->second.begin(); listIt != mapperIt->second.end(); ++listIt)
            {
                // Get each element from the list and add it into the reducer set (without duplicates)
                int elem = *listIt;
                reducerSet.insert(elem);
            }
        }
    }

    // Print the result to the output file
    ofstream fout;
    string outputFileName = "out" + to_string(reducer->exponent) + ".txt";
    fout.open(outputFileName);
    fout << reducerSet.size();
    return 0;
}

mapperArgs_T *createMapperArgs(int id, int maxExponent, vector<string> *fileNames, int *vizFileNames,
                               vector<unordered_map<int, list<int>>> *partialLists, pthread_mutex_t *mutex,
                               pthread_barrier_t *barrier)
{
    mapperArgs_T *mapperArgs = new mapperArgs_T;
    mapperArgs->id = id;
    mapperArgs->maxExponent = maxExponent;
    mapperArgs->vizFileNames = vizFileNames;
    mapperArgs->fileNames = fileNames;
    mapperArgs->partialLists = partialLists;
    mapperArgs->mutex = mutex;
    mapperArgs->barrier = barrier;
    return mapperArgs;
}

reducerArgs_T *createReducerArgs(int id, vector<unordered_map<int, list<int>>> *partialLists, int exponent,
                                 pthread_barrier_t *barrier)
{
    reducerArgs_T *reducerArgs = new reducerArgs_T;
    reducerArgs->id = id;
    reducerArgs->exponent = exponent;
    reducerArgs->partialLists = partialLists;
    reducerArgs->barrier = barrier;
    return reducerArgs;
}

int main(int argc, char **argv)
{
    ifstream fin;
    pthread_mutex_t mutex;
    pthread_barrier_t barrier;

    int noMappers = atoi(argv[1]);
    int noReducers = atoi(argv[2]);
    string fileName = argv[3];

    // Init the barrier so that the reducers will start only when all the mappers have
    // finished their work
    pthread_barrier_init(&barrier, NULL, noMappers + noReducers);
    pthread_mutex_init(&mutex, NULL);

    pthread_t mapperThreads[noMappers];
    pthread_t reducerThreads[noReducers];

    fin.open(fileName);
    int noFiles;
    fin >> noFiles;

    vector<string> fileNames;

    // Read all file names
    while (noFiles > 0)
    {
        string file;
        fin >> file;
        fileNames.push_back(file);
        noFiles--;
    }

    // Array to mark a file as allocated or unallocated
    int vizFileNames[MAX_NO_FILES] = {0};
    // Data structure to store all mappers' hashmaps
    vector<unordered_map<int, list<int>>> *partialLists = new vector<unordered_map<int, list<int>>>(noMappers);

    // Pointer to all arguments to free memory later
    mapperArgs_T *mapperArgsArr[noMappers];
    // Pointer to reducers args to free memory later
    reducerArgs_T *reducerArgsArr[noReducers];

    for (int i = 0; i < noMappers + noReducers; i++)
    {
        if (i < noMappers)
        {
            // Create mapper thread
            mapperArgsArr[i] =
                createMapperArgs(i, noReducers + 1, &fileNames, vizFileNames, partialLists, &mutex, &barrier);
            int create_status = pthread_create(&mapperThreads[i], NULL, mapper, mapperArgsArr[i]);

            if (create_status)
            {
                printf("Eroare la crearea thread-ului mapper#%d\n", i);
                exit(-1);
            }
        }
        else
        {
            // Create reducer thread
            int reducerIndex = i - noMappers;
            reducerArgsArr[reducerIndex] = createReducerArgs(reducerIndex, partialLists, reducerIndex + 2, &barrier);
            int create_status =
                pthread_create(&reducerThreads[reducerIndex], NULL, reducer, reducerArgsArr[reducerIndex]);

            if (create_status)
            {
                cout << create_status;
                printf("Eroare la crearea thread-ului reducer#%d\n", reducerIndex);
                exit(-1);
            }
        }
    }

    // Join threads and free memory
    void *status;
    for (int i = 0; i < noMappers + noReducers; i++)
    {
        if (i < noMappers)
        {
            // Mapper thread
            pthread_join(mapperThreads[i], &status);
            delete mapperArgsArr[i];
        }
        else
        {
            // Mapper reducer
            int reducerIndex = i - noMappers;
            pthread_join(reducerThreads[reducerIndex], &status);
            delete reducerArgsArr[reducerIndex];
        }
    }

    delete partialLists;
    return 0;
}