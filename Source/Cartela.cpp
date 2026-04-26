#include "Cartela.hpp"

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

namespace
{
    std::vector<int> GerarFaixaEmbaralhada(int inicio, int fim)
    {
        std::vector<int> numeros(fim - inicio + 1);
        std::iota(numeros.begin(), numeros.end(), inicio);

        std::random_device dispositivo;
        std::mt19937 gerador(dispositivo());
        std::shuffle(numeros.begin(), numeros.end(), gerador);

        return numeros;
    }
}

Cartela::Cartela()
{
    Reset();
}

void Cartela::Reset()
{
    GerarNumeros();
}

void Cartela::MarcarNumero(int numero)
{
    for (auto &linha : grade_)
    {
        for (auto &casa : linha)
        {
            if (casa.numero == numero)
            {
                casa.marcada = true;
                return;
            }
        }
    }
}

void Cartela::MarcarCelula(int linha, int coluna)
{
    if (linha < 0 || linha >= kTamanho || coluna < 0 || coluna >= kTamanho)
    {
        return;
    }

    // O espaco central ja nasce livre e marcado.
    if (linha == 2 && coluna == 2)
    {
        grade_[linha][coluna].marcada = true;
        return;
    }

    grade_[linha][coluna].marcada = !grade_[linha][coluna].marcada;
}

Cartela::VictorySnapshot Cartela::AnalyzeVictories() const
{
    VictorySnapshot snapshot{};

    bool cartelaCheia = true;
    for (const auto &linha : grade_)
    {
        for (const auto &casa : linha)
        {
            cartelaCheia = cartelaCheia && casa.marcada;
        }
    }

    for (int linha = 0; linha < kTamanho; ++linha)
    {
        bool linhaCompleta = true;
        for (int coluna = 0; coluna < kTamanho; ++coluna)
        {
            if (!grade_[linha][coluna].marcada)
            {
                linhaCompleta = false;
                break;
            }
        }

        if (linhaCompleta)
        {
            for (int coluna = 0; coluna < kTamanho; ++coluna)
            {
                snapshot.highlightedCells[linha][coluna] = true;
            }
            snapshot.linhas[linha] = true;
        }
    }

    for (int coluna = 0; coluna < kTamanho; ++coluna)
    {
        bool colunaCompleta = true;
        for (int linha = 0; linha < kTamanho; ++linha)
        {
            colunaCompleta = colunaCompleta && grade_[linha][coluna].marcada;
        }

        if (colunaCompleta)
        {
            for (int linha = 0; linha < kTamanho; ++linha)
            {
                snapshot.highlightedCells[linha][coluna] = true;
            }
            snapshot.colunas[coluna] = true;
        }
    }

    bool diagonalPrincipalCompleta = true;
    bool diagonalSecundariaCompleta = true;
    for (int indice = 0; indice < kTamanho; ++indice)
    {
        diagonalPrincipalCompleta = diagonalPrincipalCompleta && grade_[indice][indice].marcada;
        diagonalSecundariaCompleta = diagonalSecundariaCompleta && grade_[indice][kTamanho - 1 - indice].marcada;
    }

    if (diagonalPrincipalCompleta)
    {
        for (int indice = 0; indice < kTamanho; ++indice)
        {
            snapshot.highlightedCells[indice][indice] = true;
        }
        snapshot.diagonalPrincipal = true;
    }

    if (diagonalSecundariaCompleta)
    {
        for (int indice = 0; indice < kTamanho; ++indice)
        {
            snapshot.highlightedCells[indice][kTamanho - 1 - indice] = true;
        }
        snapshot.diagonalSecundaria = true;
    }

    if (cartelaCheia)
    {
        snapshot.fullCard = true;
        for (int linha = 0; linha < kTamanho; ++linha)
        {
            for (int coluna = 0; coluna < kTamanho; ++coluna)
            {
                snapshot.highlightedCells[linha][coluna] = true;
            }
        }
    }

    return snapshot;
}

const std::array<std::array<Cartela::Casa, Cartela::kTamanho>, Cartela::kTamanho> &Cartela::GetGrade() const
{
    return grade_;
}

void Cartela::GerarNumeros()
{
    constexpr std::array<int, kTamanho> faixasInicio = {1, 21, 41, 61, 81};
    constexpr std::array<int, kTamanho> faixasFim = {20, 40, 60, 80, 99};

    // Comeca sempre com a cartela limpa, preservando apenas o FREE ao final.
    for (auto &linha : grade_)
    {
        for (auto &casa : linha)
        {
            casa.numero = 0;
            casa.marcada = false;
        }
    }

    for (int coluna = 0; coluna < kTamanho; ++coluna)
    {
        auto numeros = GerarFaixaEmbaralhada(faixasInicio[coluna], faixasFim[coluna]);

        for (int linha = 0; linha < kTamanho; ++linha)
        {
            grade_[linha][coluna].numero = numeros[linha];
            grade_[linha][coluna].marcada = false;
        }
    }

    // Casa central livre para refletir a regra tradicional do bingo.
    grade_[2][2].numero = 0;
    grade_[2][2].marcada = true;
}
