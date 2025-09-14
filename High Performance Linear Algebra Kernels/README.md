# Part 3

1. Pointers can be changed to point some other places, also it can be a nullptr. It supports pointer arithmetic. However, reference has to refer to a valid object at initialization. It can not be changed to other places, and will never be null. Pointers are preferred while we pass raw airways, or we want to use memory via arithmetic. Reference can avoid copies, and also it allows direct manipulation of the variable. In our code use const double** for matrices and vectors keep the inner loop simple and also allow us to do the operation without extra copies.

---

2. Row major stores each row contiguously, and column major stores each column contiguously In our MV row major kernel. In row major storage, it stores each row of matrix in one contiguous block of memory, as we multiply by a vector, the code simply goes through the row from left to right, which means it does not need to skip around in the memory. It helps the CPU to grab data more efficiently. Similarly the column major will work best if we make the matrix stores in a column major. This is a question about how we store our matrix. For MM, the naïve A×B (both row-major) was hurt by strided access to B’s columns; transposing B (or passing Bᵀ) turned inner products into contiguous row·row dot products, improving locality and speed.

---

3. L1 L2 cache is used when we keep recent data on chip, where it will improve the latency, since it is easier to access. Spatial locality means contiguous data gets pulled into cache with each 64-byte line; temporal locality means reusing the same data soon keeps it hot. We do spatial locality by stepping through the memory one element at a time in order, where it is what we suppose to do in matrix operation(row major MV and B^T in MM), and temporal locality using x[j] across a row and reusing rows of A and b^T inside the loops.

---

4. Theoretically, alignment means placing arrays at addresses that are multiples of a boundary. Proper alignment will help avoid the split cache-line loads, and a more efficient SIMD access. Also theoretically we should expect small gains for MV and bigger gains for MM when the inner loop is contiguous. In our experiment, aligned vs. unaligned showed about 5-8 percent improvement for large row major MV and about 10-15% for MM with the B transpose having a large size.  With the naïve MM showed little change.

---

5. Inlining makes the compiler reply to function calls with the functions body, where it removes the overhead of making the call and gives the opportunity for further optimization, like the professor mentioned loop unrolling in the fast matrix program(although I am not quite sure how to do it). I think inline is the very first step to do while doing optimization.

---

6. We have realize that beside the matrix multiplication the most time-consuming part of our code is matrix and vector initialization, so one of our teammate tried to include <malloc.h> from C, to optimize the code's performance
---
7. First of all, team work helps everyone have a small chunk of code to work on, where we divide tasks by kernel. This helps with seeing different approaches and helps everyone to learn the basic logic behind matrix operation using a laptop. The collaborative analysis phase was fun, while merging benchmarks we can see how well we have done in the coding part, and it helps to visualize the pattern. Challenges encountered was mainly not sure how to use certain functions while doing benchmark and profiling, but after figuring things out, we grant a good shape in doing certain benchmarks and profiling, and will know how to optimize codes with consideration of the hardware and some algorithm etc.

PS: Windows Performance Toolkit report file size is too big to upload to github.
