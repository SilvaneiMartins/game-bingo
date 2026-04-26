#include "Cartela.hpp"
#include "Globo.hpp"

#include <raylib.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace
{
    enum class GameState
    {
        MENU,
        PLAYING,
        PAUSED,
        GAME_OVER
    };

    constexpr int kLarguraTela = 1280;
    constexpr int kAlturaTela = 830;
    constexpr const char *kTituloJanela = "BINGO MARTINS";
    constexpr int kPainelEsquerdoLargura = 470;
    constexpr int kCartelaOrigemX = 592;
    constexpr int kCabecalhoAltura = 52;
    constexpr int kTamanhoCasa = 94;
    constexpr int kEspacamentoCasa = 14;

    constexpr Color kCorFundo = {245, 241, 230, 255};
    constexpr Color kCorTextoEscuro = {28, 28, 28, 255};
    constexpr Color kCorPainel = {252, 249, 243, 255};
    constexpr Color kCorCartelaClara = {250, 250, 247, 255};
    constexpr Color kCorMarcada = {128, 0, 0, 255};
    constexpr Color kCorDestaqueVitoria = {196, 150, 42, 255};
    constexpr Color kCorBorda = {210, 201, 188, 255};
    constexpr Color kCorBotao = {128, 0, 0, 255};
    constexpr Color kCorBotaoHover = {154, 24, 24, 255};
    constexpr Color kCorBottomBar = {236, 228, 213, 255};
    constexpr int kAlturaBottomBar = 74;
    constexpr int kMargemBottomBar = 18;
    constexpr int kFolgaCartelaBottomBar = 26;

    struct LayoutCartela
    {
        Rectangle areaHeader{};
        Rectangle areaCabecalho{};
        std::array<std::array<Rectangle, Cartela::kTamanho>, Cartela::kTamanho> celulas{};
    };

    struct UIButton
    {
        Rectangle area{};
        const char *texto = "";
    };

    struct VictoryTracker
    {
        std::array<bool, Cartela::kTamanho> linhas{};
        std::array<bool, Cartela::kTamanho> colunas{};
        bool diagonalPrincipal = false;
        bool diagonalSecundaria = false;
        bool fullCard = false;
        Cartela::HighlightGrid highlightGrid{};
        int partialWins = 0;
    };

    enum class VictoryBanner
    {
        None,
        Row,
        Column,
        Diagonal,
        FullCard
    };

    LayoutCartela CriarLayoutCartela()
    {
        LayoutCartela layout{};
        const int larguraGrade = Cartela::kTamanho * kTamanhoCasa + (Cartela::kTamanho - 1) * kEspacamentoCasa;
        const int alturaGrade = Cartela::kTamanho * kTamanhoCasa + (Cartela::kTamanho - 1) * kEspacamentoCasa;
        const int alturaHeader = 76;
        const int headerX = kCartelaOrigemX;
        const int headerY = 20;
        const int headerWidth = 744;
        const int topoCartela = headerY + alturaHeader + 28;
        const int bottomBarTopo = kAlturaTela - kAlturaBottomBar - kMargemBottomBar;
        const int alturaTotalBloco = kCabecalhoAltura + 18 + alturaGrade;
        const int origemYMaximo = bottomBarTopo - kFolgaCartelaBottomBar - alturaTotalBloco;
        const int origemY = std::max(topoCartela, origemYMaximo);

        layout.areaHeader = {
            static_cast<float>(headerX),
            static_cast<float>(headerY),
            static_cast<float>(headerWidth),
            static_cast<float>(alturaHeader)};

        layout.areaCabecalho = {
            static_cast<float>(kCartelaOrigemX),
            static_cast<float>(origemY),
            static_cast<float>(larguraGrade),
            static_cast<float>(kCabecalhoAltura)};

        for (int linha = 0; linha < Cartela::kTamanho; ++linha)
        {
            for (int coluna = 0; coluna < Cartela::kTamanho; ++coluna)
            {
                layout.celulas[linha][coluna] = {
                    static_cast<float>(kCartelaOrigemX + coluna * (kTamanhoCasa + kEspacamentoCasa)),
                    static_cast<float>(origemY + kCabecalhoAltura + 18 + linha * (kTamanhoCasa + kEspacamentoCasa)),
                    static_cast<float>(kTamanhoCasa),
                    static_cast<float>(kTamanhoCasa)};
            }
        }

        return layout;
    }

    UIButton CriarBotaoCentral(float y, float largura, float altura, const char *texto)
    {
        return {
            {(kLarguraTela - largura) / 2.0f,
             y,
             largura,
             altura},
            texto};
    }

    bool BotaoFoiClicado(const UIButton &botao)
    {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            return false;
        }

        return CheckCollisionPointRec(GetMousePosition(), botao.area);
    }

    void DesenharBotao(const UIButton &botao)
    {
        const bool hover = CheckCollisionPointRec(GetMousePosition(), botao.area);
        const Color cor = hover ? kCorBotaoHover : kCorBotao;
        DrawRectangleRounded(botao.area, 0.22f, 8, cor);
        DrawRectangleRoundedLines(botao.area, 0.22f, 8, 2.0f, kCorTextoEscuro);

        const int tamanhoFonte = 30;
        const int larguraTexto = MeasureText(botao.texto, tamanhoFonte);
        DrawText(
            botao.texto,
            static_cast<int>(botao.area.x + botao.area.width / 2.0f - larguraTexto / 2.0f),
            static_cast<int>(botao.area.y + botao.area.height / 2.0f - tamanhoFonte / 2.0f - 2),
            tamanhoFonte,
            WHITE);
    }

    const char *ObterMensagemVitoria(VictoryBanner tipo)
    {
        switch (tipo)
        {
        case VictoryBanner::Row:
            return "Linha Completa!";
        case VictoryBanner::Column:
            return "Coluna Concluida!";
        case VictoryBanner::Diagonal:
            return "Diagonal Completa!";
        case VictoryBanner::FullCard:
            return "BINGO! VITORIA TOTAL!";
        case VictoryBanner::None:
        default:
            return "";
        }
    }

    VictoryTracker CriarVictoryTracker()
    {
        return {};
    }

    VictoryBanner AplicarNovasVitorias(const Cartela::VictorySnapshot &snapshot, VictoryTracker &tracker)
    {
        VictoryBanner banner = VictoryBanner::None;

        for (int linha = 0; linha < Cartela::kTamanho; ++linha)
        {
            if (snapshot.linhas[linha] && !tracker.linhas[linha])
            {
                tracker.linhas[linha] = true;
                ++tracker.partialWins;
                banner = VictoryBanner::Row;
            }
        }

        for (int coluna = 0; coluna < Cartela::kTamanho; ++coluna)
        {
            if (snapshot.colunas[coluna] && !tracker.colunas[coluna])
            {
                tracker.colunas[coluna] = true;
                ++tracker.partialWins;
                banner = VictoryBanner::Column;
            }
        }

        if (snapshot.diagonalPrincipal && !tracker.diagonalPrincipal)
        {
            tracker.diagonalPrincipal = true;
            ++tracker.partialWins;
            banner = VictoryBanner::Diagonal;
        }

        if (snapshot.diagonalSecundaria && !tracker.diagonalSecundaria)
        {
            tracker.diagonalSecundaria = true;
            ++tracker.partialWins;
            banner = VictoryBanner::Diagonal;
        }

        for (int linha = 0; linha < Cartela::kTamanho; ++linha)
        {
            for (int coluna = 0; coluna < Cartela::kTamanho; ++coluna)
            {
                tracker.highlightGrid[linha][coluna] = tracker.highlightGrid[linha][coluna] || snapshot.highlightedCells[linha][coluna];
            }
        }

        if (snapshot.fullCard && !tracker.fullCard)
        {
            tracker.fullCard = true;
            banner = VictoryBanner::FullCard;
        }

        return banner;
    }

    void DesenharCabecalhoPainelEsquerdo()
    {
        const char *titulo = "BINGO MARTINS";
        const char *instrucao = "Pressione ESPACO";

        const int tituloFonte = 40;
        const int instrucaoFonte = 24;
        const int centroX = kPainelEsquerdoLargura / 2;

        const int tituloLargura = MeasureText(titulo, tituloFonte);
        const int instrucaoLargura = MeasureText(instrucao, instrucaoFonte);

        DrawText(titulo, centroX - (tituloLargura / 2), 28, tituloFonte, kCorTextoEscuro);
        DrawText(instrucao, centroX - (instrucaoLargura / 2), 78, instrucaoFonte, kCorTextoEscuro);
    }

    void DesenharUltimoNumero(int ultimoNumero)
    {
        const char *titulo = "Ultimo Numero";
        const int centroX = kPainelEsquerdoLargura / 2;
        const int larguraTitulo = MeasureText(titulo, 28);

        DrawText(titulo, centroX - (larguraTitulo / 2), 138, 28, kCorTextoEscuro);
        DrawCircle(centroX, 264, 88.0f, kCorMarcada);

        const std::string texto = (ultimoNumero > 0) ? std::to_string(ultimoNumero) : "--";
        const int larguraTexto = MeasureText(texto.c_str(), 42);
        DrawText(texto.c_str(), centroX - (larguraTexto / 2), 243, 42, WHITE);
    }

    void DesenharHistorico(const std::vector<int> &historico)
    {
        const char *titulo = "Historico";
        const int larguraTitulo = MeasureText(titulo, 28);
        const int centroX = kPainelEsquerdoLargura / 2;
        DrawText(titulo, centroX - (larguraTitulo / 2), 386, 28, kCorTextoEscuro);

        const int quantidade = static_cast<int>(historico.size());
        const Rectangle areaHistorico = {48.0f, 430.0f, 352.0f, 250.0f};

        if (quantidade == 0)
        {
            DrawText("Nenhum numero sorteado", 104, 540, 20, kCorTextoEscuro);
            return;
        }

        int melhorColunas = 1;
        int melhorLinhas = quantidade;
        float melhorRaio = 6.0f;

        for (int colunas = 1; colunas <= quantidade; ++colunas)
        {
            const int linhas = static_cast<int>(std::ceil(static_cast<float>(quantidade) / static_cast<float>(colunas)));
            const float cellW = areaHistorico.width / static_cast<float>(colunas);
            const float cellH = areaHistorico.height / static_cast<float>(linhas);
            const float raio = std::min(cellW, cellH) * 0.34f;

            if (raio > melhorRaio)
            {
                melhorRaio = raio;
                melhorColunas = colunas;
                melhorLinhas = linhas;
            }
        }

        const float cellW = areaHistorico.width / static_cast<float>(melhorColunas);
        const float cellH = areaHistorico.height / static_cast<float>(melhorLinhas);
        const float raio = std::max(6.0f, std::min(12.0f, melhorRaio));

        for (int indice = 0; indice < quantidade; ++indice)
        {
            const int coluna = indice % melhorColunas;
            const int linha = indice / melhorColunas;
            const float centroCirculoX = areaHistorico.x + cellW * coluna + cellW / 2.0f;
            const float centroCirculoY = areaHistorico.y + cellH * linha + cellH / 2.0f;
            const std::string texto = std::to_string(historico[indice]);

            DrawCircle(static_cast<int>(centroCirculoX), static_cast<int>(centroCirculoY), raio, WHITE);
            DrawCircleLines(static_cast<int>(centroCirculoX), static_cast<int>(centroCirculoY), raio, kCorBorda);

            const int tamanhoFonte = (texto.size() >= 2) ? 12 : 13;
            const int larguraTexto = MeasureText(texto.c_str(), tamanhoFonte);
            DrawText(
                texto.c_str(),
                static_cast<int>(centroCirculoX - larguraTexto / 2.0f),
                static_cast<int>(centroCirculoY - tamanhoFonte / 2.0f - 1.0f),
                tamanhoFonte,
                BLACK);
        }
    }

    void DesenharHeaderPainelDireito(const LayoutCartela &layout, int vitoriasParciais, bool bingoCompleto)
    {
        DrawRectangleRounded(layout.areaHeader, 0.12f, 8, kCorPainel);
        DrawRectangleRoundedLines(layout.areaHeader, 0.12f, 8, 2.0f, kCorBorda);

        const float secao = layout.areaHeader.width / 3.0f;
        const int fonte = 18;
        const float centroY = layout.areaHeader.y + layout.areaHeader.height / 2.0f;
        const std::string blocos[3] = {
            "Usuario: Silvanei Martins",
            "Vitorias Parciais: " + std::to_string(vitoriasParciais),
            "Status Global: " + std::string(bingoCompleto ? "Ganhou" : "0")};

        for (int indice = 0; indice < 3; ++indice)
        {
            const float centroX = layout.areaHeader.x + secao * indice + secao / 2.0f;
            const int larguraTexto = MeasureText(blocos[indice].c_str(), fonte);
            DrawText(
                blocos[indice].c_str(),
                static_cast<int>(centroX - larguraTexto / 2.0f),
                static_cast<int>(centroY - fonte / 2.0f),
                fonte,
                indice == 2 && bingoCompleto ? kCorMarcada : kCorTextoEscuro);
        }
    }

    void DesenharCartela(const Cartela &cartela, const LayoutCartela &layout, const Cartela::HighlightGrid &destaqueVitoria)
    {
        static constexpr char titulos[Cartela::kTamanho] = {'B', 'I', 'N', 'G', 'O'};
        const auto &grade = cartela.GetGrade();

        for (int coluna = 0; coluna < Cartela::kTamanho; ++coluna)
        {
            const std::string titulo(1, titulos[coluna]);
            const Rectangle cabecalho = {
                layout.areaCabecalho.x + coluna * (kTamanhoCasa + kEspacamentoCasa),
                layout.areaCabecalho.y,
                static_cast<float>(kTamanhoCasa),
                static_cast<float>(kCabecalhoAltura)};

            const int tituloFonte = 32;
            const int larguraTexto = MeasureText(titulo.c_str(), tituloFonte);
            DrawText(
                titulo.c_str(),
                static_cast<int>(cabecalho.x + cabecalho.width / 2.0f - larguraTexto / 2.0f),
                static_cast<int>(cabecalho.y + 10),
                tituloFonte,
                kCorTextoEscuro);
        }

        for (int linha = 0; linha < Cartela::kTamanho; ++linha)
        {
            for (int coluna = 0; coluna < Cartela::kTamanho; ++coluna)
            {
                const auto &casa = grade[linha][coluna];
                const Rectangle celula = layout.celulas[linha][coluna];
                const bool destacar = destaqueVitoria[linha][coluna];
                const Color corFundo = destacar ? kCorDestaqueVitoria : (casa.marcada ? kCorMarcada : kCorCartelaClara);
                const Color corTexto = casa.marcada ? WHITE : BLACK;

                DrawRectangleRounded(celula, 0.12f, 8, corFundo);
                DrawRectangleRoundedLines(celula, 0.12f, 8, 2.0f, kCorBorda);

                std::string texto = std::to_string(casa.numero);
                if (linha == 2 && coluna == 2)
                {
                    texto = "FREE";
                }

                const int tamanhoFonte = (texto == "FREE") ? 24 : 30;
                const int larguraTexto = MeasureText(texto.c_str(), tamanhoFonte);
                DrawText(
                    texto.c_str(),
                    static_cast<int>(celula.x + celula.width / 2.0f - larguraTexto / 2.0f),
                    static_cast<int>(celula.y + celula.height / 2.0f - tamanhoFonte / 2.0f - 3),
                    tamanhoFonte,
                    corTexto);
            }
        }
    }

    void DesenharBottomBar(GameState estado, const std::string &mensagem, const UIButton &botaoContinuar)
    {
        const Rectangle area = {18.0f, static_cast<float>(kAlturaTela - kAlturaBottomBar - kMargemBottomBar), 1244.0f, static_cast<float>(kAlturaBottomBar)};
        DrawRectangleRounded(area, 0.16f, 8, kCorBottomBar);
        DrawRectangleRoundedLines(area, 0.16f, 8, 2.0f, kCorBorda);
        DrawRectangleRoundedLines(
            {area.x + 2.0f, area.y + 2.0f, area.width - 4.0f, area.height - 4.0f},
            0.16f,
            8,
            1.0f,
            Color{250, 245, 236, 255});

        if (!mensagem.empty())
        {
            DrawText(mensagem.c_str(), static_cast<int>(area.x + 24.0f), static_cast<int>(area.y + 16.0f), 28, kCorTextoEscuro);
        }

        if (estado == GameState::PAUSED)
        {
            DrawText("Conferencia de Vitoria", static_cast<int>(area.x + 24.0f), static_cast<int>(area.y + 44.0f), 24, kCorMarcada);
            DesenharBotao(botaoContinuar);
        }
    }

    void DesenharFundoJogo()
    {
        ClearBackground(kCorFundo);

        // Painel esquerdo para separar a area do globo da cartela.
        DrawRectangleRounded({18.0f, 18.0f, 430.0f, 684.0f}, 0.04f, 8, kCorPainel);
        DrawRectangleRoundedLines({18.0f, 18.0f, 430.0f, 684.0f}, 0.04f, 8, 2.0f, kCorBorda);
    }

    void DesenharFundoMenu()
    {
        ClearBackground(kCorFundo);
    }

    void DesenharMenu(const UIButton &botaoStart)
    {
        const char *subtitulo = "Clique para iniciar uma nova rodada";
        const int larguraTitulo = MeasureText("BINGO MARTINS", 56);
        const int larguraSubtitulo = MeasureText(subtitulo, 26);
        const int centroX = kLarguraTela / 2;
        const int blocoTopo = 210;

        DrawText("BINGO MARTINS", centroX - larguraTitulo / 2, blocoTopo, 56, kCorTextoEscuro);
        DrawText(subtitulo, centroX - larguraSubtitulo / 2, blocoTopo + 78, 26, kCorTextoEscuro);
        DesenharBotao(botaoStart);
    }

    void ProcessarCliqueNaCartela(Cartela &cartela, const LayoutCartela &layout)
    {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            return;
        }

        const Vector2 mouse = GetMousePosition();

        for (int linha = 0; linha < Cartela::kTamanho; ++linha)
        {
            for (int coluna = 0; coluna < Cartela::kTamanho; ++coluna)
            {
                if (CheckCollisionPointRec(mouse, layout.celulas[linha][coluna]))
                {
                    cartela.MarcarCelula(linha, coluna);
                    return;
                }
            }
        }
    }
}

int main()
{
    InitWindow(kLarguraTela, kAlturaTela, kTituloJanela);
    SetTargetFPS(60);

    Globo globo;
    Cartela cartela;
    const LayoutCartela layoutCartela = CriarLayoutCartela();
    const UIButton botaoStart = CriarBotaoCentral(338.0f, 240.0f, 72.0f, "START");
    const UIButton botaoContinuar = {{936.0f, static_cast<float>(kAlturaTela - 68), 314.0f, 42.0f}, "Continuar Sorteio"};

    GameState estado = GameState::MENU;
    VictoryTracker victoryTracker = CriarVictoryTracker();
    VictoryBanner bannerAtual = VictoryBanner::None;
    std::string mensagemBottomBar = "Clique em START para iniciar.";

    const auto reiniciarPartida = [&]()
    {
        globo.Reset();
        cartela.Reset();
        victoryTracker = CriarVictoryTracker();
        bannerAtual = VictoryBanner::None;
        mensagemBottomBar = "Partida iniciada. Pressione ESPACO para sortear.";
        estado = GameState::PLAYING;
    };

    while (!WindowShouldClose())
    {
        if (estado == GameState::MENU && BotaoFoiClicado(botaoStart))
        {
            reiniciarPartida();
        }

        if (estado == GameState::PLAYING)
        {
            if (IsKeyPressed(KEY_SPACE) && globo.TemNumerosDisponiveis())
            {
                const int numeroSorteado = globo.Sortear();
                cartela.MarcarNumero(numeroSorteado);

                const auto snapshot = cartela.AnalyzeVictories();
                const VictoryBanner novaVitoria = AplicarNovasVitorias(snapshot, victoryTracker);
                if (novaVitoria != VictoryBanner::None)
                {
                    bannerAtual = novaVitoria;
                    mensagemBottomBar = ObterMensagemVitoria(novaVitoria);

                    if (novaVitoria == VictoryBanner::FullCard)
                    {
                        estado = GameState::GAME_OVER;
                        mensagemBottomBar = "BINGO! VITORIA TOTAL! - Pressione R para voltar ao menu";
                    }
                    else
                    {
                        estado = GameState::PAUSED;
                    }
                }
                else
                {
                    mensagemBottomBar = "Numero sorteado: " + std::to_string(numeroSorteado);
                }

                if (!globo.TemNumerosDisponiveis() && estado == GameState::PLAYING)
                {
                    estado = GameState::GAME_OVER;
                    mensagemBottomBar = "Fim de jogo: todos os 99 numeros foram sorteados. Pressione R para voltar ao menu.";
                }
            }

            ProcessarCliqueNaCartela(cartela, layoutCartela);

            const auto snapshotClique = cartela.AnalyzeVictories();
            const VictoryBanner novaVitoriaClique = AplicarNovasVitorias(snapshotClique, victoryTracker);
            if (novaVitoriaClique != VictoryBanner::None)
            {
                bannerAtual = novaVitoriaClique;
                mensagemBottomBar = ObterMensagemVitoria(novaVitoriaClique);

                if (novaVitoriaClique == VictoryBanner::FullCard)
                {
                    estado = GameState::GAME_OVER;
                    mensagemBottomBar = "BINGO! VITORIA TOTAL! - Pressione R para voltar ao menu";
                }
                else
                {
                    estado = GameState::PAUSED;
                }
            }
        }

        if (estado == GameState::PAUSED && BotaoFoiClicado(botaoContinuar))
        {
            estado = GameState::PLAYING;
            bannerAtual = VictoryBanner::None;
            mensagemBottomBar = "Pressione ESPACO para continuar.";
        }

        if (estado == GameState::GAME_OVER && IsKeyPressed(KEY_R))
        {
            globo.Reset();
            cartela.Reset();
            victoryTracker = CriarVictoryTracker();
            bannerAtual = VictoryBanner::None;
            estado = GameState::MENU;
            mensagemBottomBar = "Clique em START para jogar novamente.";
        }

        if (estado == GameState::MENU)
        {
            BeginDrawing();
            DesenharFundoMenu();
            DesenharMenu(botaoStart);
            DesenharBottomBar(estado, mensagemBottomBar, botaoContinuar);
            EndDrawing();
            continue;
        }

        BeginDrawing();
        DesenharFundoJogo();

        DesenharCabecalhoPainelEsquerdo();
        DesenharUltimoNumero(globo.GetUltimoNumero());
        DesenharHistorico(globo.GetDrawHistory());
        DesenharHeaderPainelDireito(layoutCartela, victoryTracker.partialWins, victoryTracker.fullCard);
        DesenharCartela(cartela, layoutCartela, victoryTracker.highlightGrid);

        DesenharBottomBar(estado, mensagemBottomBar, botaoContinuar);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
