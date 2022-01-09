#!/usr/bin/env zsh

test -f isr.S && rm isr.S
cp isr.S.base isr.S

for i ({0..255}); do
  echo "isr$i:" >> isr.S
  echo "  push ebp" >> isr.S
  echo "  mov ebp, esp" >> isr.S
  if (( !(i == 8 || i == 17 || (i >= 10 && i <= 14) ) )); then
    echo "  push 0" >> isr.S
  fi
  echo "  push $i" >> isr.S
  echo "  jmp catchall" >> isr.S
done

x=""
for i ({0..255}); do 
  if [ $i = 255 ]; then 
    x="$x isr$i"
  else
    x="$x isr$i,"
  fi
done

echo "isrs dd $x" >> isr.S