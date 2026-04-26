#ifndef GLOBO_HPP
#define GLOBO_HPP

#include <random>
#include <vector>

class Globo
{
public:
    Globo();

    int Sortear();
    bool TemNumerosDisponiveis() const;
    void Reset();

    const std::vector<int> &GetDrawHistory() const;
    int GetUltimoNumero() const;

private:
    static constexpr int kNumeroMaximo = 99;

    std::vector<int> numerosDisponiveis_;
    std::vector<int> drawnNumbers_;
    int ultimoNumero_;
    std::mt19937 gerador_;
};

#endif
