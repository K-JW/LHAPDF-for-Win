<#
    Author: KANG, Jin-Wen
    E-Mail: kngjinwen@mails.ccnu.edu.cn
    Date:   2020-05-26 21:39 +0800
    Desc:   Get the source of LHAPDF
    Usage:  Please specify the required version number first,
            You can look it up in: 
                https://lhapdf.hepforge.org/downloads/
#>

curl https://lhapdf.hepforge.org/downloads/?f=LHAPDF-6.2.3.tar.gz `
    -o LHAPDF.tar.gz
mkdir LHAPDF
tar -xzvf LHAPDF.tar.gz -C ./LHAPDF --strip-components 1
rm LHAPDF.tar.gz
echo "OK!"