#include <stdio.h>

void build_heap(int nums[], int len);
void adjust_heap(int nums[], int len);
void heap_sort(int nums[], int len);
void show_array(int nums[], int len);

int main()
{
  int nums[] = {2, 3, 1, 9, 5, 6, 8, 7, 4};
  int len = 9;
  heap_sort(nums, len);
  show_array(nums, len);
}

void build_heap(int nums[], int len)
{
  int mid = len / 2;
  while (mid > 0)
  {
  }
}

void heap_sort(int nums[], int len)
{
  
}

void show_array(int nums[], int len)
{
  int i = 0;
  for (i = 0; i < len; ++i)
  {
    printf("%d\t", nums[i]);
  }
  printf("\n");
}
