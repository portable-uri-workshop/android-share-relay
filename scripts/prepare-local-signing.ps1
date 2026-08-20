[CmdletBinding()]
param(
    [string]$Keytool = "keytool"
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$privateDirectory = Join-Path $projectRoot "_private"
$keystorePath = Join-Path $privateDirectory "kakaolink-bridge-release.jks"
$propertiesPath = Join-Path $projectRoot "keystore.properties"

if ((Test-Path -LiteralPath $keystorePath) -or
    (Test-Path -LiteralPath $propertiesPath)) {
    throw "Local signing files already exist. Refusing to overwrite them."
}

New-Item -ItemType Directory -Path $privateDirectory | Out-Null
$passwordBytes = [Security.Cryptography.RandomNumberGenerator]::GetBytes(24)
$password = [Convert]::ToBase64String($passwordBytes).Replace("+", "-").Replace("/", "_").TrimEnd("=")

& $Keytool `
    -genkeypair `
    -keystore $keystorePath `
    -storepass $password `
    -keypass $password `
    -alias "kakaolink-bridge" `
    -keyalg RSA `
    -keysize 4096 `
    -validity 10000 `
    -dname "CN=KakaoLink Bridge, OU=Release, O=KakaoLink Bridge" `
    -noprompt

if ($LASTEXITCODE -ne 0) {
    throw "keytool failed with exit code $LASTEXITCODE"
}

$properties = @(
    "storeFile=_private/kakaolink-bridge-release.jks"
    "storePassword=$password"
    "keyAlias=kakaolink-bridge"
    "keyPassword=$password"
)
[IO.File]::WriteAllLines($propertiesPath, $properties, [Text.UTF8Encoding]::new($false))

Write-Host "Created local release signing files. They are excluded by .gitignore."
