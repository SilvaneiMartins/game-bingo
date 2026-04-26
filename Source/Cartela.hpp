#ifndef CARTELA_HPP
#define CARTELA_HPP

#include <array>

class Cartela
{
public:
    static constexpr int kTamanho = 5;
    using HighlightGrid = std::array<std::array<bool, kTamanho>, kTamanho>;

    struct VictorySnapshot
    {
        std::array<bool, kTamanho> linhas {};
        std::array<bool, kTamanho> colunas {};
        bool diagonalPrincipal = false;
        bool diagonalSecundaria = false;
        bool fullCard = false;
        HighlightGrid highlightedCells {};
    };

    struct Casa
    {
        int numero = 0;
        bool marcada = false;
    };

    Cartela();

    void MarcarNumero(int numero);
    void MarcarCelula(int linha, int coluna);
    void Reset();
    VictorySnapshot AnalyzeVictories() const;

    const std::array<std::array<Casa, kTamanho>, kTamanho> &GetGrade() const;

private:
    using Grade = std::array<std::array<Casa, kTamanho>, kTamanho>;

    Grade grade_;

    void GerarNumeros();
};

#endif
