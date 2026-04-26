#include "Globo.hpp"

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>

Globo::Globo()
    : ultimoNumero_(0),
      gerador_(std::random_device{}())
{
    Reset();
}

void Globo::Reset()
{
    numerosDisponiveis_.resize(kNumeroMaximo);
    std::iota(numerosDisponiveis_.begin(), numerosDisponiveis_.end(), 1);
    drawnNumbers_.clear();
    ultimoNumero_ = 0;
}

int Globo::Sortear()
{
    if (numerosDisponiveis_.empty())
    {
        throw std::runtime_error("Nao ha mais numeros disponiveis para sorteio.");
    }

    // Embaralha os numeros restantes e consome sempre o ultimo elemento.
    std::shuffle(numerosDisponiveis_.begin(), numerosDisponiveis_.end(), gerador_);

    ultimoNumero_ = numerosDisponiveis_.back();
    numerosDisponiveis_.pop_back();
    drawnNumbers_.push_back(ultimoNumero_);

    return ultimoNumero_;
}

bool Globo::TemNumerosDisponiveis() const
{
    return !numerosDisponiveis_.empty();
}

const std::vector<int> &Globo::GetDrawHistory() const
{
    return drawnNumbers_;
}

int Globo::GetUltimoNumero() const
{
    return ultimoNumero_;
}
