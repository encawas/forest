@echo off
setlocal EnableExtensions

cd /d "%~dp0.."

set "BASE=hardware_reference\s3_base_project_20260710.zip.b64"
set "COMBINED=%BASE%.combined"
set "OUTPUT=hardware_reference\s3_base_project_20260710.zip"
set "EXPECTED_SHA256=a75ff2ef026ee1d97d8b7d01eeab1fadea44af19e34c6fab8bc4668fa4ab3453"

for %%P in (00 01 02a 02b 02c 02d 03 04 05 06 07) do (
  if not exist "%BASE%.part%%P" (
    echo [FAIL] Missing %BASE%.part%%P
    exit /b 1
  )
)

if exist "%COMBINED%" del /q "%COMBINED%"
if exist "%OUTPUT%" del /q "%OUTPUT%"

copy /b "%BASE%.part00"+"%BASE%.part01"+"%BASE%.part02a"+"%BASE%.part02b"+"%BASE%.part02c"+"%BASE%.part02d"+"%BASE%.part03"+"%BASE%.part04"+"%BASE%.part05"+"%BASE%.part06"+"%BASE%.part07" "%COMBINED%" >nul
if errorlevel 1 (
  echo [FAIL] Could not combine S3 reference parts.
  exit /b 1
)

certutil -f -decode "%COMBINED%" "%OUTPUT%" >nul
if errorlevel 1 (
  echo [FAIL] certutil could not decode the S3 reference archive.
  del /q "%COMBINED%"
  exit /b 1
)

del /q "%COMBINED%"

for /f "tokens=*" %%H in ('certutil -hashfile "%OUTPUT%" SHA256 ^| findstr /r /v "hash CertUtil"') do (
  set "ACTUAL_SHA256=%%H"
  goto :hash_read
)

:hash_read
set "ACTUAL_SHA256=%ACTUAL_SHA256: =%"

if /i not "%ACTUAL_SHA256%"=="%EXPECTED_SHA256%" (
  echo [FAIL] SHA-256 mismatch.
  echo Expected: %EXPECTED_SHA256%
  echo Actual:   %ACTUAL_SHA256%
  del /q "%OUTPUT%"
  exit /b 1
)

echo [PASS] Decoded and verified:
echo %OUTPUT%
exit /b 0
