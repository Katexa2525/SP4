#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

#define MAX_THREADS 4

using namespace std;


typedef struct ThreadData
{
  string line;
  string end_line;
  //int check;
} MYDATA, * PMYDATA;

DWORD WINAPI ThreadSortFunction(LPVOID lpParam);
void merge(vector<char>&, int, int, int);
void mergeSort(vector<char>&, int, int);

int main()
{
  PMYDATA pDataArray[MAX_THREADS];
  DWORD dwThreadIdArray[MAX_THREADS];
  HANDLE hThreadArray[MAX_THREADS];

  vector<string> pool;
  string result;
  fstream fs;


  // Проверяем существует ли файл
  fs.open("test2.txt", ios::in);
  if (fs.is_open())
  {
    string line;
    // Читаем строки из файла и помещаем их в вектор 
    while (getline(fs, line))
    {
      pool.push_back(line);
    }
    fs.close();
  }
  else
  {
    cout << "File doesn't exist";
    return -1;
  }

  for (int i = 0, arraysIdx = 0; i < pool.size(); i++, arraysIdx++)
  {
    // Выделение памяти под структуру данных для потока
    pDataArray[arraysIdx] = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));

    if (pDataArray[arraysIdx] == NULL)
    {
      ExitProcess(2);
    }

    // Заполнение структуры данными для сортировки
    pDataArray[arraysIdx]->line = string(pool[i]);

    // Создание потока сортировки
    hThreadArray[arraysIdx] = CreateThread(
      NULL,                       // атрибуты безопасности по умолчанию
      0,                          // использовать размер стека по умолчанию
      ThreadSortFunction,         // имя функции потока
      pDataArray[arraysIdx],      // аргументы для функции потока
      0,                          // использовать флаги создания по умолчанию
      &dwThreadIdArray[arraysIdx] // возвращает идентификатор потока по умолчанию
    );

    if (hThreadArray[arraysIdx] == NULL)
    {
      ExitProcess(3);
    }

    // Если достигнуто максимальное количество потоков, ожидание их завершения
    if (arraysIdx == MAX_THREADS - 1)
    {
      WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);
      // Обработка завершенных потоков
      for (int j = 0; j <= arraysIdx; j++)
      {
        CloseHandle(hThreadArray[j]);
        if (pDataArray[j] != NULL)
        {
          result += string(pDataArray[j]->end_line);
          HeapFree(GetProcessHeap(), 0, pDataArray[j]);
          pDataArray[j] = NULL;  // Гарантия, что адрес не будет повторно использован 
        }
      }
      // Сброс индекса для следующей группы потоков
      arraysIdx = -1;
    }
  }

  vector<char> vec_result(result.begin(), result.end());
  mergeSort(vec_result, 0, result.length() - 1);
  result = string(vec_result.begin(), vec_result.end());
  fs.open("result.txt", ios::out);
  if (fs.is_open())
  {
    string line;
    fs << result;
    fs.close();
  }

  return 0;
}

DWORD WINAPI ThreadSortFunction(LPVOID lpParam)
{
  PMYDATA pDataArray;

  string tmp_line;

  pDataArray = (PMYDATA)lpParam;

  tmp_line = pDataArray->line;
  sort(tmp_line.begin(), tmp_line.end());
  pDataArray->end_line = string(tmp_line);

  return 0;
}


void merge(vector<char>& arr, int l, int m, int r)
{
  int i, j, k;
  int n1 = m - l + 1;
  int n2 = r - m;

  vector<char> L(n1), R(n2);

  // Заполняем временные массивы L и R данными из основного массива arr
  for (i = 0; i < n1; i++)
    L[i] = arr[l + i];
  for (j = 0; j < n2; j++)
    R[j] = arr[m + 1 + j];

  // Объединяем временные массивы L и R обратно в основной массив arr
  i = 0;
  j = 0;
  k = l;
  while (i < n1 && j < n2)
  {
    if (L[i] <= R[j])
    {
      arr[k] = L[i];
      i++;
    }
    else
    {
      arr[k] = R[j];
      j++;
    }
    k++;
  }
  // Дописываем оставшиеся элементы из L (если они есть)
  while (i < n1)
  {
    arr[k] = L[i];
    i++;
    k++;
  }
  // Дописываем оставшиеся элементы из R (если они есть)
  while (j < n2)
  {
    arr[k] = R[j];
    j++;
    k++;
  }
}

void mergeSort(vector<char>& arr, int l, int r)
{
  if (l >= r)
  {
    return; // Уже отсортировано (базовый случай рекурсии)
  }
  int m = l + (r - l) / 2;
  // Рекурсивно сортируем левую и правую половины
  mergeSort(arr, l, m);
  mergeSort(arr, m + 1, r);
  // Объединяем отсортированные половины
  merge(arr, l, m, r);
}