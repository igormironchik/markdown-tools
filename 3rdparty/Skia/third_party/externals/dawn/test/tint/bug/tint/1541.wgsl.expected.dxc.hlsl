
[numthreads(1, 1, 1)]
void main() {
  bool a = true;
  bool v = select(false, true, (a & true));
}

