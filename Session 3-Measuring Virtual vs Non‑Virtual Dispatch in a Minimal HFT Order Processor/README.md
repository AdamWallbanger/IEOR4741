CPU Model: M1
ProductName:		macOS
ProductVersion:		14.1.2
BuildVersion:		23B92

Apple clang version 15.0.0 (clang-1500.1.0.2.5)
Target: arm64-apple-darwin23.1.0
Thread model: posix
InstalledDir: /Library/Developer/CommandLineTools/usr/bin

Pattern,Non-virtual (median ops/s),Virtual (median ops/s),Gap %
homogeneous,341000500,197211000,72.9%
mixed,174978000,92042600,90.1%
bursty,347272500,190325000,82.5%

Homogeneous
.4×10^8 ops/s non-virtual and 2.0×10^8 virtual which is about 1.7 times faster
if dispatch becomes perfectly predictable and the A path can inline; the virtual path pays a vtable indirect call and blocks inlining every order.

Mixed 50/50
1.7×10^8 non-virtual and 0.9×10^8 virtual about 1.9 times faster.
random assignment hurts the non-virtual branch predictor, but virtual still loses more from indirect call + no inline.

Bursty 64A/16B
3.5×10^8 non-virtual vs 1.9×10^8 virtual about 1.8 times faster.
periodicity restores branch predictability for the non-virtual switch; virtual still can’t inline and pays the vcall cost.

I think the gap is dominated by dispatch overhead and lost inlining.

Avoiding virtual calls gives you about 1.7–1.9 time higher throughput because it enables inlining and avoids an indirect branch on the critical path while L1 keeps data hot.
