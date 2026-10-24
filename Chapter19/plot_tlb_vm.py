import matplotlib.pyplot as plt

pages = [1, 2, 4, 8, 16, 32, 64, 128,
         256, 512, 1024, 2048, 4096]

# Ubuntu VM results
trials_10000 = [
    10.30, 2.60, 3.42, 1.43, 2.40, 1.54, 2.99,
    2.24, 2.32, 1.94, 1.89, 1.33, 6.46
]

trials_100000 = [
    3.19, 2.88, 7.76, 5.87, 2.35, 2.16, 2.10,
    1.57, 1.62, 1.29, 1.22, 1.49, 6.35
]

trials_1000000 = [
    4.71, 3.09, 1.78, 1.34, 1.05, 1.02, 1.22,
    1.25, 1.15, 1.25, 1.36, 1.37, 6.14
]

plt.figure(figsize=(11, 6))

plt.plot(pages, trials_10000, marker='o', label='10,000 Trials')
plt.plot(pages, trials_100000, marker='s', label='100,000 Trials')
plt.plot(pages, trials_1000000, marker='^', label='1,000,000 Trials')

plt.xscale('log', base=2)
plt.xticks(pages, [str(p) for p in pages])

plt.xlabel('Number of Pages Accessed')
plt.ylabel('Average Access Time (ns)')
plt.title('TLB Performance – Ubuntu VM (4 KB Pages)')

plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()

plt.savefig('tlb_ubuntu.png', dpi=300)
plt.show()