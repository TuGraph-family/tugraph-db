#!/usr/bin/env python
import string
import random


def random_generator(size=10, chars=string.ascii_letters + string.digits):
    return ''.join(random.choice(chars) for x in range(size))


class DataGenerator:
    def __init__(self):
        pass

    def __encode_no(self, x, length=10):
        s = str(x)
        while len(s) < length:
            s = "0" + s
        return s

    # length        length of no
    def __random_no(self, count_v, length=10):
        return self.__encode_no(random.randint(0, count_v - 1), length)

    # length        length of name
    def __random_name(self, length=10):
        return random_generator(length)

    # count_v       number of vertexes
    def test_write_vertex(self, count_v, output):
        for i in range(count_v):
            no = self.__encode_no(i)
            name = self.__random_name()
            output.write(no + "," + name + "\n")

    # count_src     number of source vertexes
    # count_dst     number of destination vertexes
    # count_e       number of edges
    def test_write_edge(self, count_src, count_dst, count_e, output):
        for i in range(count_e):
            no_from = self.__random_no(count_src)
            no_to = self.__random_no(count_dst)
            output.write(no_from + "," + no_to + "\n")


if __name__ == "__main__":
    dg = DataGenerator()

    f = open("person.csv", "w")
    f.write("LABEL=person\n"
            "no:STRING:ID,name:STRING\n")
    count_person = 1 << 20;
    dg.test_write_vertex(count_person, f)
    f.close()

    f = open("knows.csv", "w")
    f.write("LABEL=knows,SRC_LABEL=person,DST_LABEL=person\n"
            "no:STRING:SRC_ID,no:STRING:DST_ID\n")
    dg.test_write_edge(count_person, count_person, count_person * 10, f)
    f.close()
