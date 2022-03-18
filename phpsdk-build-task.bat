@ECHO OFF
SET currDir=%CD%
SET LOG_NAME=%currDir%\Build_20220316_1932.log
@CALL phpsdk_buildtree phpdev > %LOG_NAME% 2>&1
IF NOT EXIST php-8.1.3-src @CALL git clone -b php-8.1.3 --depth 1 --single-branch https://github.com/php/php-src.git php-8.1.3-src
CD php-8.1.3-src
SET phpSrc=%CD%
@CALL phpsdk_deps -u >> %LOG_NAME% 2>&1
@CALL ROBOCOPY %currDir%\Source\sqlsrv %phpSrc%\ext\sqlsrv /s /xx /xo
@CALL ROBOCOPY %currDir%\Source\shared %phpSrc%\ext\sqlsrv\shared /s /xx /xo
@CALL ROBOCOPY %currDir%\Source\pdo_sqlsrv %phpSrc%\ext\pdo_sqlsrv /s /xx /xo
@CALL ROBOCOPY %currDir%\Source\shared %phpSrc%\ext\pdo_sqlsrv\shared /s /xx /xo
@CALL buildconf --force >> %LOG_NAME% 2>&1
@CALL cscript configure.js --disable-all --enable-cli --enable-cgi --enable-json --enable-embed --enable-sqlsrv=shared --enable-pdo --with-pdo-sqlsrv=shared  --enable-debug  >> %LOG_NAME% 2>&1
nmake >> %LOG_NAME% 2>&1
exit
