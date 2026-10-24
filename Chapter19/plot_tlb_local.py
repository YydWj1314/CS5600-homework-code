import matplotlib.pyplot as plt

pages = [1, 2, 4, 8, 16, 32, 64, 128,
         256, 512, 1024, 2048, 4096]

# Local macOS results (16 KB page size)
trials_10000 = [
    4.00, 4.80, 1.30, 1.18, 4.93, 3.22, 36.33,
    11.19, 3.87, 4.84, 4.46, 5.30, 7.54
]

trials_100000 = [
    4.32, 4.48, 3.26, 1.69, 2.28, 3.57, 4.28,
    6.34, 3.77, 3.81, 4.59, 4.60, 6.91
]

trials_1000000 = [
    9.45, 3.44, 1.66, 1.38, 2.72, 3.12, 3.43,
    3.69, 4.04, 3.93, 3.99, 4.50, 6.26
]

plt.figure(figsize=(11, 6))

plt.plot(pages, trials_10000, marker='o', label='10,000 Trials')
plt.plot(pages, trials_100000, marker='s', label='100,000 Trials')
plt.plot(pages, trials_1000000, marker='^', label='1,000,000 Trials')

plt.xscale('log', base=2)
plt.xticks(pages, [str(p) for p in pages])

plt.xlabel('Number of Pages Accessed')
plt.ylabel('Average Access Time (ns)')
plt.title('TLB Performance – Local macOS (16 KB Pages)')

plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()

plt.savefig('tlb_local.png', dpi=300)
plt.show()