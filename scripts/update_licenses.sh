for f in `grep -d recurse -l QT_BEGIN_LICENSE ./src`
do
    mv $f $f.orig
    perl update_license.pl < $f.orig > $f
    rm $f.orig
done
