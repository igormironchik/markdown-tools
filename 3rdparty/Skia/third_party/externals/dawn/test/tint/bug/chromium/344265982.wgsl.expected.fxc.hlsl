
RWByteAddressBuffer buffer : register(u0);
void foo() {
  {
    int i = int(0);
    for( ; (i < int(4)); i = asint((asuint(i) + asuint(int(1))))) {
      bool tint_continue = false;
      switch(asint(buffer.Load((0u + (uint(i) * 4u))))) {
        case int(1):
        {
          tint_continue = true;
          break;
        }
        default:
        {
          buffer.Store((0u + (uint(i) * 4u)), asuint(int(2)));
          break;
        }
      }
      if (tint_continue) {
        continue;
      }
    }
  }
}

void main() {
  foo();
}

