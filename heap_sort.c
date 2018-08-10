#include <stdio.h>
#include <assert.h>

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

// 建堆，将任意一个数组调整为堆
void build_heap(int nums[], int len)
{
  int mid = len / 2 - 1;
  for (; mid >= 0; mid--)
  {
    adjust_heap(nums, len, mid);
  }
}

// 调整堆，让其满足堆的特性
void adjust_heap(int nums[], int len, int adj_index)
{
  assert(adj_index >= 0);
  int child = 0;
  for (child = 2 * adj_index; child < len; child = child * 2 + 1)
  {
    // 先比较左右孩子谁大谁小，再将大的和父节点比较
    int r_child = child + 1;
    if (rchild < len && nums[rchild] > nums [child])
      child = r_child;
    
    if (nums[child] > nums [adj_index])
    {
      int tmp = nums[adj_index];
      nums[adj_index] = nums[child];
      nums[child] = tmp;
      adj_index = child;
    }
  }
}

void heap_sort(int nums[], int len)
{
  assert(len > 0);
  int index = len;
  int tmp = 0;
  build_heap(nums, len);
  
  for (; index > 0; index--)
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
