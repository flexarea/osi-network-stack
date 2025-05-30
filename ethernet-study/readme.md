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
1st round: 61.77, 63.46, 61.25
2nd round: 64.00 60.66, 60.71
3rd round: 63.94, 62.4, 61.19
4th round: 63.98, 61.07, 61.11
5th round: 62.46, 63.17, 61.47

If we take the average for each input value of N, we get:

N=10 -> (61.77 + 64.00 + 63.94 + 63.98 + 62.46) / 5 = 63.23%
N=20 -> (63.46 + 60.66 + 62.4 + 61.07 + 63.17) / 5 = 62.15%
N=40 -> (61.25 + 60.71 + 61.19 + 61.11 + 61.47) / 5 = 61.15%

Conclusion:
----------

- From the results, we observe that as the number of connected devices (N) increases, the success rate slightly decreases. However, the decline is not drastic, suggesting that while more devices may cause additional collisions, the system maintains a relatively stable performance.

- The lower transmission success rate might also stem from the assumption that each device has a 50% chance of attempting to send a frame in any given time slot. This high probability significantly impacts performance. Since devices randomly select a destination port, the likelihood of multiple devices choosing the same port increases with the number of devices (N), leading to a higher chance of collisions and a reduced success rate.

- We assume the switch operates over a full-duplex transmission medium, as our simulation does not check whether the destination device is transmitting. This assumption reduces the likelihood of collisions compared to a half-duplex system, where simultaneous transmission and reception are not possible, leading to a significantly lower transmission success rate.

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

Conclusion:
----------

- Given the recorded outputs above we see that for N=10 nearly all transmissions fail due to collisions. And as we scale N, things seem to even get worse as the hub blindly forwards all transmissions to all connected devices, resulting in a higher collision rate.

- Similar to the switch, the implementation of this simulation significantly affects the low transmission success rate, mostly due to the assumption that each device has a 50% chance of transmitting a frame in any given time slot. This high probability drastically reduces the success rate, especially in a hub-based setup where frames are broadcast to all devices. Since simultaneous transmissions from two or more devices lead to immediate collisions, the 50% transmission probability greatly increases the likelihood of collisions as the number of devices (N) grows, making it nearly impossible to avoid collisions at scale.

- We assume a half-duplex transmission medium for the hub, as devices cannot transmit and receive simultaneously. This constraint allows collisions to occur when two or more devices attempt to send frames over the shared medium at the same time.
