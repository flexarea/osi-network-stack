# I. Binary Exponential Backoff
## The following are outputs I got when running the program 3 times with different input values:
### first round

Input: 10
Output: 63
------------
Input: 20
Output:113
------------
Input: 40
Output: 197
------------
Input: 80
Output: 427

### second round

Input: 10
Output: 32
------------
Input: 20
Output:96
------------
Input: 40
Output: 200
------------
Input: 80
Output: 578

### third round

Input: 10
Output: 52
------------
Input: 20
Output: 77
------------
Input: 40
Output: 181
------------
Input: 80
Output: 462

## Observation

It appears that the output scales approximately linearly with the input. For example, in the first round, the output for N = 10 is roughly half of the output for N = 20. Similarly, when N = 20, its output is about half of the output for N = 40. Specifically, 113*2 = 226, which is only 29 more than the output of 197 for  N = 40.

In the second round, when N = 20, the output is 96. Doubling the input to N = 40 results in an output of 200, which is very close to twice the previous value. Specifically, 96 × 2 = 192, meaning the actual output differs by only 8, further reinforcing the near-linear scaling pattern.

## Conclusion
From this observation, we can conclude that:

1. The output almost always doubles when the input N is doubled.
2. However, the output does not perfectly double or scale with N by the same factor. Instead, it fluctuates slightly, either more or less. This variability is likely due to the influence of a randomly generated number within the program, which introduces some level of randomness into the output.

# II. Hubs vs Switches
# a. Switch

I got the following output in percentage after I ran the swith program simulation 5 times with the input N=10, N=20, N=40 respectively:
1st round: 28.5, 8.79, 0.83
2nd round: 29.1, 9.04, 0.75
3rd round: 28.05, 7.61, 0.72
4th round: 27.98, 10.07, 1.11
5th round: 28.46, 10.17, 1.1

If we take the average for each input value of N we get:
N=10 -> (28.5+29.1+28.05+27.98+28.46)/5 = 28.42%
N=20 -> (8.79+9.04+7.61+10.07+10.17)/5 = 9.14%
N=40 -> (0.83+0.75+0.72+1.11+1.1)/5 = 0.9%

Conclusion: It seems like as N increases or more devices connected to the switch, the success rate decreases, likely due to more collisions.

# b. Hub

I got the following output in percentage after I ran the hub program simulation 5 times with the input N=10, N=20, N=40 respectively:
1st round: 0.24, 0.0, 0.0
2nd round: 0.15, 0.0, 0.0
3rd round: 0.22, 0.0, 0.0
4th round: 0.24, 0.0, 0.0
5th round: 0.26, 0.0, 0.0

If we take the average for each input value of N we get:
N=10 -> (0.24+0.15+0.22+0.24+0.26)/5 = 0.22%
N=20 -> 0.0
N=40 -> 0.0

Conclusion: Given the recorded outputs above we see that for N=10 nearly all transmissions fail due to collisions. And as we scale N, things seem to even get worse as the hub blindly forwards all transmissions to all connected devices, resulting in a higher collision rate.
