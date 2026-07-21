SKIP: INVALID

%f = @fragment func(%fbf:f16 [@color(0)]):f16 [@location(0)] {
  $B1: {
    ret %fbf
  }
}
Failed to generate: @color attribute is not supported by the HLSL backend

tint executable returned error: exit status 1
