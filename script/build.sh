STARDUST=/mnt/c/Users/zache/OneDrive/Desktop/Ninja/Trailblazing10/files

make -j${nproc}
echo 'Build finsihed. Copying REL to Trailblazing 10'
cp mkb2.rel_sample.rel "$STARDUST"
echo 'Done.'