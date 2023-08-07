eps=$1
python3 main_dgl_reddit_gat.py --runs 1 --epochs $eps --num-layers 2 --num-hidden 256 --eval
