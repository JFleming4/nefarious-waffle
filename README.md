# Assignment 3

@author Derek Stride 100955939
@author Justin Fleming 100934170

## Execution

To Run the project first build it with make and then run it by doing one of the following

```bash
./scheduler
./scheduler num_of_process
```

## Dynamic Priority Calculations

Please see lines 315 - 317 in scheduler.c for the implementation of the following algorithm

The Bonus is a minimum of 0 or the previous value subtract the run-time / 200 where the previous value ranges from
0 - 10

The dynamic priority is then calulated using the given formula of max (100, min (139, previous - bonus + 5) )
