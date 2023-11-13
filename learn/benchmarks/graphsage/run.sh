eps=5
python3 train_full.py --dataset reddit --gpu 0 --n-epochs $eps --n-layers 2 --n-hidden 256 --aggregator-type gcn
