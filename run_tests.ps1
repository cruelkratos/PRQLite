param(
    [switch]$NoBuild
)

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Definition
Push-Location $scriptRoot

$buildDir = "build\tests"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$commonSrcs = @(
    "src\frontend\parser\parser.cpp",
    "src\frontend\parser\AST.cpp",
    "src\frontend\lexer.cpp",
    "src\table.cpp",
    "src\storage.cpp",
    "src\catalog.cpp" 
)

function Compile-Test($testFile) {
    $exe = Join-Path $buildDir ([IO.Path]::GetFileNameWithoutExtension($testFile) + ".exe")
    if ($NoBuild) { return $exe }

    if (Get-Command g++ -ErrorAction SilentlyContinue) {
        $args = @('-std=c++17','-Iinclude','-I.') + $testFile + $commonSrcs + @('-O0','-g','-o',$exe)
        & g++ @args
    }
    elseif (Get-Command cl -ErrorAction SilentlyContinue) {
        $all = @($testFile) + $commonSrcs
        $clArgs = @('/std:c++17','/EHsc','/I','include','/I','.') + $all + @('/Fe:' + $exe)
        & cl @clArgs
    }
    else {
        Write-Error "No supported compiler found (g++ or cl)."
        exit 1
    }

    return $exe
}

$tests = Get-ChildItem -Path tests -Filter test_*.cpp | Sort-Object Name
$failures = @()

foreach ($t in $tests) {
    Write-Output "=== Building $($t.Name) ==="
    $exe = Compile-Test $t.FullName
    if (-not (Test-Path $exe)) { Write-Error "Build failed: $($t.Name)"; $failures += $t.Name; continue }

    Write-Output "=== Running $($t.Name) ==="
    $out = & $exe 2>&1 | Out-String
    Write-Output $out
    if ($out -match '\[FAIL\]') { $failures += $t.Name }
}

if ($failures.Count -eq 0) {
    Write-Output "All tests completed (no '[FAIL]' lines detected)."
    exit 0
}
else {
    Write-Output "Tests with failures: $($failures -join ', ')"
    exit 1
}

Pop-Location
