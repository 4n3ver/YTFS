import requests
import os
import getpass

import shutil

import tagger

USER = None
HOME = None
MOUNT = '/tmp/ytfs'
ALBUMS = MOUNT + '/Albums'
DECADES = MOUNT + '/Decades/0'
TEMP = None


class SorryIHave5ProjectsDueOverTheBreak(Exception):
    pass


def main(ip, port):
    serveraddr = ip + str(port)

    # get the invoking username, hopefully its the user we want not the ROOT
    global USER
    USER = getpass.getuser()

    global HOME, TEMP
    HOME = '/home/' + USER + '.ytfsmusic'
    TEMP = HOME + '/.temp'


def __test_env():
    if not os.path.exists(MOUNT):
        raise SorryIHave5ProjectsDueOverTheBreak

    # let the hack begin...
    if not os.path.exists(TEMP):
        os.makedirs(TEMP)


def set_up():
    __test_env()

    global USER
    USER = getpass.getuser()

    global HOME, TEMP
    HOME = '/home/' + USER + '/.ytfsmusic'
    TEMP = HOME + '/.temp'


def hack_tag():
    for thing in os.listdir(DECADES):
        thing = str(thing)
        lie_albums = DECADES + '/' + thing
        for stuff in os.listdir(lie_albums):
            stuff = str(stuff)
            temp_stuff = TEMP + '/' + stuff
            lie_decades = lie_albums + '/' + stuff
            print(lie_decades)
        if os.path.isfile(lie_decades):
            print('copying: ' + HOME + '/' + stuff + ' -> ' + temp_stuff)
            shutil.copyfile(HOME + '/' + stuff, temp_stuff)
            print('tagging: ' + temp_stuff)
            tagger.lookup(temp_stuff)
            print('removing: ' + lie_decades)
            os.remove(lie_decades)
            print('moving: ' + temp_stuff + ' -> ' + MOUNT)
            shutil.move(temp_stuff, MOUNT)
    for thing in os.listdir(ALBUMS):
        thing = str(thing)
        lie_albums = ALBUMS + '/' + thing
        temp_thing = TEMP + '/' + thing
        print(lie_albums)
        if os.path.isfile(lie_albums):
            print('copying: ' + HOME + '/' + thing + ' -> ' + temp_thing)
            shutil.copyfile(HOME + '/' + thing, temp_thing)
            print('tagging: ' + temp_thing)
            tagger.lookup(temp_thing)
            print('removing: ' + lie_albums)
            os.remove(lie_albums)
            print('moving: ' + temp_thing + ' -> ' + MOUNT)
            shutil.move(temp_thing, MOUNT)


if __name__ == '__main__':
    from sys import argv

    if len(argv) == 3:
        main(ip=argv[1], port=argv[2])
