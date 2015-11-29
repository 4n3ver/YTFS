""" lots of dependencies for few lines of codes...
git clone https://bitbucket.org/acoustid/chromaprint.git
sudo add-apt-repository ppa:mc3man/trusty-media
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install ffmpeg python3-pip libfftw3-dev libfftw3-doc
pip3 install pyacoustid mutagen
cd chromaprint
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF

usage python3 tagger.py filename
"""
import threading

import acoustid
import re
from mutagen.easyid3 import EasyID3, mutagen

__version__ = '0.0a'

ACOUSTID_KEY = 'ROzOUsl1'
KEY = 'cSpUJKpD'

year = None
artist = None
album = None
title = None


def lookup(path):
    path = str(path)
    a = threading.Thread(
        target=_lookup_album,
        kwargs={'path': path}
    )
    b = threading.Thread(
        target=_look_title,
        kwargs={'path': path}
    )
    b.start()
    a.start()

    try:
        file = EasyID3(path)
    except mutagen.id3.ID3NoHeaderError:
        file = mutagen.File(path, easy=True)
        file.add_tags()

    b.join()
    if 'title' not in file and title:
        file['title'] = title

    a.join()
    if 'album' not in file and album:
        file['album'] = album
    if 'artist' not in file and artist:
        file['artist'] = artist
    if 'date' not in file and year:
        file['date'] = year

    file.save()
    print(str(file))
    print(str(title) + ' ' + str(year) + ' ' + str(artist) + ' ' + str(album))


def _look_title(path):
    global title
    res = str(acoustid.match(
        apikey=KEY,
        path=path,
        parse=False,
        meta=['recordings']
    ))
    title = re.search('\'results\':.*?\'title\'\: \'(.+?)\'', res)
    if title:
        title = title.group(1)
    return title


def _lookup_album(path):
    global year
    global artist
    global album

    res = str(acoustid.match(
        apikey=ACOUSTID_KEY,
        path=path,
        parse=False,
        meta=['releases', 'releasegroups']
    ))

    year = re.search('\'releaseevents\'\:.*?\'year\'\: ([0-9]{4})', res)
    if year:
        year = year.group(1)

    artist = re.search('\'artists\'\:.*?\'name\'\: \'(.+?)\'', res)
    if artist:
        artist = artist.group(1)

    album = re.search('\'releases\'\:.*?\'title\'\: \'(.+?)\'', res)
    if album:
        album = album.group(1)
    return year, artist, album


if __name__ == '__main__':
    from sys import argv

    if len(argv) == 2:
        lookup(argv[1])
