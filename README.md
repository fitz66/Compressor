Compressor
==========

Compressor takes is a c program that takes a text file as a commandline argument and compresses it.

Decompressor is a c program (not written by me) that decompresses the compressed file Compressor created.

To compress a file type ./compress textfile.txt | ./decompress

compare original and the new decompressed: ./compress textfile.txt | ./decompress | diff - textfile.txt

also,

./compress textfile.txt > textfile.txt.zipped
creates a file that is takes up less memory than the original.
