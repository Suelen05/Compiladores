$root = Split-Path -Parent $PSScriptRoot
$exePath = Join-Path $root "exec\microcompilador.exe"

if (-not (Test-Path $exePath)) {
    Write-Error "Executável não encontrado em $exePath. Compile com 'nmake' antes de rodar os testes."
}

$tests = @(
    @{ Name="tokens_err_lexico"; Mode="--tokens"; Input="tests\err_lexico.txt"; Expected="tests\tokens_out\err_lexico.tokens.out" },
    @{ Name="tokens_ok_basico"; Mode="--tokens"; Input="tests\ok_basico.txt"; Expected="tests\tokens_out\ok_basico.tokens.out" },
    @{ Name="tokens_err_sintatico"; Mode="--tokens"; Input="tests\err_sintatico.txt"; Expected="tests\tokens_out\err_sintatico.tokens.out" },
    @{ Name="tokens_err_semantico_undeclarado"; Mode="--tokens"; Input="tests\err_semantico_undeclarado.txt"; Expected="tests\tokens_out\err_semantico_undeclarado.tokens.out" },
    @{ Name="tokens_err_semantico_tipo"; Mode="--tokens"; Input="tests\err_semantico_tipo.txt"; Expected="tests\tokens_out\err_semantico_tipo.tokens.out" },
    @{ Name="tokens_err_semantico_if"; Mode="--tokens"; Input="tests\err_semantico_if.txt"; Expected="tests\tokens_out\err_semantico_if.tokens.out" },

    @{ Name="ast_ok_basico"; Mode="--ast"; Input="tests\ok_basico.txt"; Expected="tests\ast_out\ok_basico.ast.out" },
    @{ Name="ast_err_sintatico"; Mode="--ast"; Input="tests\err_sintatico.txt"; Expected="tests\ast_out\err_sintatico.ast.out" },
    @{ Name="ast_err_sem_undeclarado"; Mode="--ast"; Input="tests\err_semantico_undeclarado.txt"; Expected="tests\ast_out\err_semantico_undeclarado.ast.out" },
    @{ Name="ast_err_sem_tipo"; Mode="--ast"; Input="tests\err_semantico_tipo.txt"; Expected="tests\ast_out\err_semantico_tipo.ast.out" },
    @{ Name="ast_err_sem_if"; Mode="--ast"; Input="tests\err_semantico_if.txt"; Expected="tests\ast_out\err_semantico_if.ast.out" },

    @{ Name="run_ok_basico"; Mode="--run"; Input="tests\ok_basico.txt"; Expected="tests\run_out\ok_basico.run.out" },
    @{ Name="run_err_sem_undeclarado"; Mode="--run"; Input="tests\err_semantico_undeclarado.txt"; Expected="tests\run_out\err_semantico_undeclarado.run.out" },
    @{ Name="run_err_sem_tipo"; Mode="--run"; Input="tests\err_semantico_tipo.txt"; Expected="tests\run_out\err_semantico_tipo.run.out" },
    @{ Name="run_err_sem_if"; Mode="--run"; Input="tests\err_semantico_if.txt"; Expected="tests\run_out\err_semantico_if.run.out" }
)

function Normalize([string]$text) {
    return ($text -replace "`r`n","`n")
}

$summary = @()

foreach ($t in $tests) {
    $expectedPath = Join-Path $root $t.Expected
    if (-not (Test-Path $expectedPath)) {
        $summary += [pscustomobject]@{ Test=$t.Name; Mode=$t.Mode; Status="MISS_EXPECTED"; Detail="Arquivo esperado não encontrado" }
        continue
    }

    $tmp = New-TemporaryFile

    $actualLinesRaw = & $exePath $t.Mode $t.Input 2>&1
    $exitCode = $LASTEXITCODE
    Set-Content -Path $tmp -Value ($actualLinesRaw -join "`r`n")

    $expectedLines = Get-Content $expectedPath
    $actualLines   = Get-Content $tmp

    # ignora diferenças de comentários (acentuação) nos testes de tokens
    if ($t.Mode -eq "--tokens") {
        $expectedLines = $expectedLines | Where-Object { $_ -notmatch '^COMMENTARIO' }
        $actualLines   = $actualLines   | Where-Object { $_ -notmatch '^COMMENTARIO' }
    }
    $diff = Compare-Object $expectedLines $actualLines -SyncWindow 2

    if ($diff.Count -eq 0) {
        $summary += [pscustomobject]@{ Test=$t.Name; Mode=$t.Mode; Status="PASS"; Detail="" }
    } else {
        $actualOutPath = "$expectedPath.actual"
        Set-Content -NoNewline -Path $actualOutPath -Value (Get-Content -Raw $tmp)

        $diffText = ($diff | ForEach-Object { "$($_.SideIndicator) $($_.InputObject)" }) -join "`n"

        $summary += [pscustomobject]@{
            Test   = $t.Name
            Mode   = $t.Mode
            Status = "FAIL"
            Detail = "Diferenças em $actualOutPath`n" + $diffText
        }
    }

    Remove-Item $tmp -ErrorAction SilentlyContinue
}

Write-Host "`nResumo:"
$summary | Format-Table -AutoSize

Write-Host "`nDetalhes de falha:"
$summary | Where-Object { $_.Status -ne "PASS" } | ForEach-Object {
    Write-Host "[$($_.Test) / $($_.Mode)] $($_.Detail)`n"
}
