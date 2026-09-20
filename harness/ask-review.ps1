<##
.SYNOPSIS
Antigravity 설계 제안을 Codex 읽기 전용 검수로 전달한다.

.DESCRIPTION
프로젝트 루트의 GEMINI.md 지침을 따르도록 Antigravity CLI에 요청한 뒤,
반환된 Gemini Proposal을 Codex CLI의 표준 입력으로 전달한다.
Codex는 읽기 전용 sandbox에서 제안과 현재 저장소를 독립적으로 검수한다.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateNotNullOrEmpty()]
    [string]$Question
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot

function Get-RequiredCommand
{
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue

    if ($null -eq $command)
    {
        throw "'$Name' 명령을 찾을 수 없습니다. 설치 상태와 PATH를 확인하세요."
    }

    return $command
}

if (-not (Test-Path -LiteralPath (Join-Path $projectRoot 'GEMINI.md')))
{
    throw "프로젝트 지침 파일을 찾을 수 없습니다: $projectRoot\GEMINI.md"
}

$null = Get-RequiredCommand -Name 'agy'
$null = Get-RequiredCommand -Name 'codex'

$geminiPrompt = @"
현재 작업 위치는 Gimbal Framework 프로젝트 루트다.

프로젝트 루트의 GEMINI.md를 먼저 읽고 모든 지침을 따라라.
특히 AGENTS.md, 요청과 관련된 Source/Header, 관련 Skill을 확인한 뒤,
GEMINI.md의 'Gemini Proposal' 출력 형식으로 설계 제안서만 작성해라.

파일을 수정하거나 빌드, Test 실행, Flash 다운로드, 보드 실행을 하지 마라.
Shell 명령을 실행하거나 권한을 요청하지 마라. Workspace의 읽기 전용 파일 도구만 사용해라.
서브에이전트, Background Task, 결과 대기를 사용하지 마라.
이번 응답 안에서 완성된 Gemini Proposal을 출력해라. 작업 중간 상태나 대기 메시지만 출력해서는 안 된다.

사용자 요청:
$Question
"@

Write-Host '[1/2] Antigravity가 Gemini Proposal을 생성합니다.'
Push-Location -LiteralPath $projectRoot

try
{
   # 여기에 --dangerously-skip-permissions 플래그를 추가합니다.
    $antigravityOutput = & agy -p $geminiPrompt --output-format json --dangerously-skip-permissions
    $antigravityExitCode = $LASTEXITCODE
}
finally
{
    Pop-Location
}

$geminiProposal = $null

try
{
    if ($antigravityExitCode -ne 0)
    {
        throw "종료 코드: $antigravityExitCode"
    }

    $antigravityJson = $antigravityOutput -join [Environment]::NewLine

    if ([string]::IsNullOrWhiteSpace($antigravityJson))
    {
        throw '응답 본문이 비어 있습니다.'
    }

    $antigravityResult = $antigravityJson | ConvertFrom-Json -ErrorAction Stop

    if ($null -eq $antigravityResult)
    {
        throw 'JSON 응답이 비어 있습니다.'
    }

    $statusProperty = $antigravityResult.PSObject.Properties['status']
    $responseProperty = $antigravityResult.PSObject.Properties['response']
    $errorProperty = $antigravityResult.PSObject.Properties['error']

    if (($null -eq $statusProperty) -or ($null -eq $responseProperty))
    {
        throw 'JSON 응답에 status 또는 response가 없습니다.'
    }

    $status = $statusProperty.Value
    $response = $responseProperty.Value
    $errorMessage = if ($null -eq $errorProperty) { $null } else { $errorProperty.Value }

    if (($status -ne 'SUCCESS') -or [string]::IsNullOrWhiteSpace($response))
    {
        if ([string]::IsNullOrWhiteSpace($errorMessage))
        {
            $errorMessage = '응답 본문이 비어 있습니다.'
        }

        throw $errorMessage
    }

    $geminiProposal = $response
}
catch
{
    $fallbackReason = $_.Exception.Message
    Write-Warning "Antigravity 제안서를 사용할 수 없습니다. Codex 단독 검수로 계속합니다: $fallbackReason"
    $geminiProposal = @"
# Gemini Proposal

## 상태
- Antigravity 제안서를 사용할 수 없음: $fallbackReason

## Codex 요청
- 사용자 요청과 실제 저장소를 기준으로 독립 답변이 필요함.
"@
}

$codexPrompt = @"
당신은 Gimbal Framework의 Codex 검수자다.

프로젝트 루트의 AGENTS.md와 요청에 맞는 .agents/skills/*/SKILL.md를 먼저 읽고,
현재 저장소 Source/Header를 직접 확인해라. 표준 입력에는 Antigravity가 생성한 Gemini Proposal이 들어 있다.
제안서를 사실로 신뢰하지 말고 독립적으로 검수해라.

이 실행은 검수 전용이다. 파일 수정, 빌드, Test 실행, Flash 다운로드, 보드 실행을 하지 마라.

사용자 요청:
$Question

표준 입력의 제안서가 비어 있거나 작업 중간 상태라면, 그 이유만으로 반려하지 마라.
대신 사용자 요청과 실제 저장소를 기준으로 독립적으로 조사해 요청에 대한 답변을 완성해라.

한국어로 다음 형식으로 출력해라.

# Codex Review

## 판정
- 승인 / 조건부 승인 / 반려 중 하나와 짧은 이유

## 요청에 대한 답변
- 사용자 요청에 직접 답한다.
- 수정 파일 목록, 변경 목적, Interface 영향, Hardware 확인 항목을 구체적으로 작성한다.

## 확인된 사실
- 실제 저장소 파일을 근거로 확인한 내용

## 제안서 검수 결과
- Antigravity 제안에서 맞는 점, 누락된 점, 잘못된 점

## 구현 전 필요 사항
- 사용자 결정, 추가 조사, 변경 범위 또는 가정

## 검증 계획
- 구현이 명시적으로 요청된 경우 수행할 Build, Test, Board 검증
"@

Write-Host '[2/2] Codex가 Antigravity 제안서를 읽기 전용으로 검수합니다.'
$geminiProposal | & codex exec -C $projectRoot -s read-only $codexPrompt

if ($LASTEXITCODE -ne 0)
{
    throw "Codex 검수에 실패했습니다. 종료 코드: $LASTEXITCODE"
}
