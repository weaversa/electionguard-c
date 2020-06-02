int bar(int *v) {
  return *v + 1;
}

int foo()
{
  int v;
  return bar(&v);
}
