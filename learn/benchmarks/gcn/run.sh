eps=50
python3 train.py --dataset reddit --gpu 0 --self-loop --n-epochs ${eps} --n-layers 2 --n-hidden 256
