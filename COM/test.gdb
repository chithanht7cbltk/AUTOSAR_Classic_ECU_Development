target remote localhost:3334
continue &
shell sleep 4
interrupt
print RxState
quit
