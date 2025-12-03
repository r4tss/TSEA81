import numpy as np
import matplotlib.pyplot as plt

N1 = np.array([10, 20, 30, 40, 50, 60, 70, 80, 90])
N2 = np.array([10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34])

# All times are in ms
unoptimized_pthreads = np.array([1308, 4678, 9934, 18096, 26868, 37373, 49766, 63132, 77221])

unoptimized_messages = np.array([287, 572, 836, 1130, 1427, 1742, 2107, 2511, 2942, 3395, 3877, 4384, 4925])

optimized_pthreads = np.array([802, 1676, 2759, 4161, 6069, 8525, 11028, 13909, 17242])

optimized_messages = np.array([114, 232, 357, 489, 634, 789, 954, 1131, 1321, 1582, 1746, 1977, 2221])

plt.subplot(2, 2, 1)
plt.plot(N1, unoptimized_pthreads)
plt.title("Unoptimized pthreads")
plt.xlabel("Number of persons")
plt.ylabel("Time (ms)")

plt.subplot(2, 2, 2)
plt.plot(N2, unoptimized_messages)
plt.title("Unoptimized messages")
plt.xlabel("Number of persons")
plt.ylabel("Time (ms)")

plt.subplot(2, 2, 3)
plt.plot(N1, optimized_pthreads)
plt.title("Optimized pthreads")
plt.xlabel("Number of persons")
plt.ylabel("Time (ms)")

plt.subplot(2, 2, 4)
plt.plot(N2, optimized_messages)
plt.title("Optimized messages")
plt.xlabel("Number of persons")
plt.ylabel("Time (ms)")

plt.tight_layout()

plt.show()

n_vip_0 = np.array([746, 1857, 3268, 5200, 8891, 12221, 16616, 24165, 31745])
n_vip_1 = np.array([741, 1769, 3034, 5168, 7354, 11583, 13960, 19097, 24764])
n_vip_2 = np.array([734, 1781, 3141, 5455, 10077, 13488, 17866, 24028, 31550])
n_vip_3 = np.array([731, 1760, 3092, 5217, 8075, 11624, 17931, 22573, 28854])
n_vip_4 = np.array([731, 1745, 3047, 4601, 8195, 11781, 16386, 20792, 28022])
n_vip_5 = np.array([720, 1725, 3016, 4742, 8375, 11746, 16251, 21268, 27699])

plt.plot(N1, n_vip_0)
plt.plot(N1, n_vip_1)
plt.plot(N1, n_vip_2)
plt.plot(N1, n_vip_3)
plt.plot(N1, n_vip_4)
plt.plot(N1, n_vip_5)

plt.title("Travel times with n VIPs")
plt.xlabel("Number of people")
plt.ylabel("Time (ms)")

plt.legend(["n=0", "n=1", "n=2", "n=3", "n=4", "n=5"])

plt.show()
