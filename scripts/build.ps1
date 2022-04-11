#!/usr/bin/env pwsh

Set-StrictMode -Version latest
$ErrorActionPreference = "Stop"

$workspaceFolder = Resolve-Path "${PSScriptRoot}/.."
$workspaceFolderBasename = Split-Path $workspaceFolder -Leaf
$cwd = Get-Location
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

$buildDirectory = Get-VScodeSetting "cmake.buildDirectory"
if (-Not (Test-Path $buildDirectory)) {
	New-Item $buildDirectory -ItemType Directory | Out-Null
}

$cmakeDefines = Build-CMakeArguments

$cmakeCacheFile = "$buildDirectory/CMakeCache.txt"
if (-Not (Test-Path $cmakeCacheFile) -or -Not (Test-Path "$buildDirectory/CMakeFiles/Makefile.cmake")) {
	if ($IsWindows) {
		$cmakeGenerator = "MinGW Makefiles"
	} else {
		$cmakeGenerator = "Unix Makefiles"
	}
	$debugFile = "$buildDirectory/cmake.trace.log"
	Invoke-CMake "-DCMAKE_BUILD_TYPE:STRING=Debug" "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE" @cmakeDefines -S $workspaceFolder -B $buildDirectory -G $cmakeGenerator "--trace-redirect=${debugFile}" # --debug-trycompile --debug-output --debug-find --trace-expand

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
	Remove-Item -Verbose -Path ${buildDirectory}/dist/bin/*
	Remove-Item -Verbose -Path "${buildDirectory}/gui/BurningTool${suffix}"
	Remove-Item -Verbose -Path "${buildDirectory}/test-binary/test${suffix}"
	exit $LASTEXITCODE
}
