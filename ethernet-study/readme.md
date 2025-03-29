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

It seems like the output seems to linearly scale with the input. What I mean is that we can cleary see that whenN=10 in the first round the output is closely half of the one we get after increasing N by a multiple of two. And when N=20 its output is closely half of what we get when doubling it again. Indeed, 113*2 = 226, which is 29 more than 197, the output we get for N=40.

If we look at the second round when N=20, we get 96. And after we double the input N (so N=40) we get an output that is very close to double the previous value of N; i.e. when N=20, with even a smaller difference of 8 when calculating 96*2, which is 192.

What we get from this observation is:
1. The output always almost always double when we double the input N
2. The output never perfectly double or scale with N by the same multiple. It either more or less by a factor variable. Tvaoluehis could be due to the fact that the output depend on a random generated number inside the code for the program.

## Conclusion
This looks very much like a linear equation or more precisely a slope y = 2N+b where y is the output, N the previo

