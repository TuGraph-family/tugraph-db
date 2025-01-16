import requests
import random
import time
from  multiprocessing import Pool

url = 'http://127.0.0.1:12380/add'
def send_post_request(i):
    try:
        total = 0
        for i in range(10000):
            num = random.randint(-100000, 100000)
            response = requests.post(url, data=str(num))
            if response.status_code != 200:
                print(f"error response: {response.status_code}, {response.text}", flush=True)
            total = total + num
    except Exception as e:
        print(f"Error: {e}")
    print(f"total : {total}", flush=True)
    return total

t1 = time.time()
thread_num=5
res_list=[]
pool = Pool(thread_num)
for i in range(thread_num):
    res = pool.apply_async(func=send_post_request, args=(i,))
    res_list.append(res)
final_total = 0
for res in res_list:
    final_total = final_total + res.get()
pool.close()
pool.join()
t2 = time.time()
elapsed_time = t2- t1
print(f"final total: {final_total}", flush=True)
print(f"add cost: {elapsed_time:.6f} seconds", flush=True)