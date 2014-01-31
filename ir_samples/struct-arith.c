typedef struct {
  int a, b, c, d;
} dummyvec4;


int compute_important_function(dummyvec4* v1, dummyvec4* v2) {
  int temp1 = v1->a + v2->a * v2->d;
  int temp2 = v1->b + v2->c * v2->b;
  int temp3 = v1->c * v1->b * v1->c * v1->d;
  return temp1 - temp2 - temp3;
}

