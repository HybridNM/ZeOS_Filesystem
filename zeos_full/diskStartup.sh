sudo umount fs_mount
sudo losetup -d /dev/loop0
rm fs.img
dd if=/dev/zero of=fs.img bs=512 count=10000
sudo losetup -f --show fs.img
sudo mkfs.fat -D 0 -f 1 -F 32 -S 512 -s 8 -v /dev/loop0
sudo mount /dev/loop0 fs_mount/
cd fs_mount/
sudo mkdir DIR1
sudo mkdir DIR2
cd DIR1
echo "file0 here" | sudo tee FILE0
echo "file1 here" | sudo tee FILE1
cd ..
cd DIR2
echo "file2 here" | sudo tee FILE2
