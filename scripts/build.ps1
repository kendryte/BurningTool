#!/usr/bin/env pwsh

Set-StrictMode -Version latest
$ErrorActionPreference = "Stop"

$env:HTTP_PROXY = ''
$env:HTTPS_PROXY = ''

$workspaceFolder = $(Resolve-Path "${PSScriptRoot}/..").Path
$workspaceFolderBasename = $(Split-Path $workspaceFolder -Leaf)
$cwd = $(Get-Location).Path
$defaultBuildTask = "CMake: Build"
$pathSeparator = [IO.Path]::DirectorySeparatorChar

Set-Location $workspaceFolder

$settings = Get-Content "${workspaceFolder}/.vscode/settings.json" | ConvertFrom-Json

function Get-VScodeSetting {
	param (
		[Parameter(Mandatory = $true)]
		[string] $field
	)

	return $ExecutionContext.InvokeCommand.ExpandString($settings.$field)
}
function Invoke-CMake {
	Write-Host -ForegroundColor Yellow " == cmake $args =="
	cmake @args
}

function Build-CMakeArguments {
	if (-Not( [bool]($settings.PSobject.Properties.name -match "cmake.configureSettings"))) {
		return;
	}
	$v = $settings."cmake.configureSettings"
	foreach ($key in $v.psobject.properties.name) {
		"-D${key}=$($v.$key)"
	}
}

$sourceDirectory = Get-VScodeSetting "cmake.sourceDirectory"
if (-Not (Test-Path $sourceDirectory)) {
	Write-Error "something goes wrong!"
	exit 6
}

if ($env:CI) {
	$buildDirectory = $cwd
} else {
	$buildDirectory = Get-VScodeSetting "cmake.buildDirectory"
	if (-Not (Test-Path $buildDirectory)) {
		New-Item $buildDirectory -ItemType Directory | Out-Null
	}
}

$cmakeDefines = @(Build-CMakeArguments)

if ($env:CMAKE_MAKE_PROGRAM -And (Test-Path $env:CMAKE_MAKE_PROGRAM)) {
	$cmakeDefines += "-DCMAKE_MAKE_PROGRAM=$env:CMAKE_MAKE_PROGRAM"
} elseif ($IsWindows) {
	if (Test-Path C:\msys64) {
		$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\clang64\bin;" + $env:PATH
		Write-Output "using default C:\msys64"
	}
	$mingwMake = $(Get-Command mingw32-make.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Definition)
	if (-Not($mingwMake)) {
		Write-Error "can not find mingw32-make.exe in `$PATH"
		exit 1
	}
	$mingwMake = $mingwMake -replace "\\", "/"
	$cmakeDefines += "-DCMAKE_MAKE_PROGRAM=$mingwMake"

	$mingwGcc = $mingwMake -replace [regex]::escape("mingw32-make.exe"), "gcc.exe"
	$cmakeDefines += "-DCMAKE_C_COMPILER=$mingwGcc"

	$mingwGxx = $mingwMake -replace [regex]::escape("mingw32-make.exe"), "g++.exe"
	$cmakeDefines += "-DCMAKE_CXX_COMPILER=$mingwGxx"
}

$cmakeCacheFile = "$buildDirectory/CMakeCache.txt"
if (-Not (Test-Path $cmakeCacheFile) -or -Not (Test-Path "$buildDirectory/CMakeFiles/Makefile.cmake")) {
	if ($IsWindows) {
		$cmakeGenerator = "MinGW Makefiles"
	} else {
		$cmakeGenerator = "Unix Makefiles"
	}
	$debugFile = "$buildDirectory/cmake.trace.log"
	Invoke-CMake `
		-Wno-dev `
		'-DCMAKE_BUILD_TYPE:STRING=Debug' `
		'-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE' `
		@cmakeDefines `
		-S $workspaceFolder `
		-B $buildDirectory `
		-G $cmakeGenerator `
		--trace-redirect=${debugFile}
	# --debug-trycompile
	# --debug-trycompile --debug-output --debug-find --trace-expand

	if ($LASTEXITCODE -ne 0) {
		Remove-Item $cmakeCacheFile | Out-Null
	}
}

Invoke-CMake --build $buildDirectory --parallel

if (Test-Path "${buildDirectory}/compile_commands.json") {
	$compileCommands = Get-VScodeSetting "cmake.copyCompileCommands"
	Move-Item "${buildDirectory}/compile_commands.json" -Destination $compileCommands -Force | Out-Null
}

if ($LASTEXITCODE -ne 0) {
	if ($IsWindows) {
		$suffix = '.exe'
	} else {
		$suffix = ''
	}
	Remove-Item -Verbose -Path ${buildDirectory}/dist/bin/* -ErrorAction SilentlyContinue
	Remove-Item -Verbose -Path "${buildDirectory}/gui/BurningTool${suffix}" -ErrorAction SilentlyContinue
	Remove-Item -Verbose -Path "${buildDirectory}/test-binary/test${suffix}" -ErrorAction SilentlyContinue
	exit $LASTEXITCODE
}
