# StudyComfort - Global Solution 2026.2 | Edge Computing

**Vídeo da apresentação:** [Assistir no YouTube](https://youtu.be/MziTbxpFiXo)

**Simulação:** [Abrir o Wokwi](https://wokwi.com/projects/480352166150085633)  
**Dashboard:** [Abrir o StudyComfort](https://gs-web-front-open-source-consulting.vercel.app/)

## Ficha CitizenTech

| Campo | Definição |
|---|---|
| Nome da solução | Study Comfort. |
| Cidade e bairro | São Paulo/SP |
| Cidadão beneficiado | Estudantes universitários que querem escolher um ambiente adequado para estudar, trabalhar em grupo ou gravar vídeos/projetos. |
| Problema em uma frase | Estudantes têm dificuldade para identificar, antes de se deslocar, salas com espaço e condições adequadas de ruído, temperatura e luminosidade para suas atividades. |
| Como funciona | O fluxo da S01 (Sala 1) é ESP32 simulado no Wokwi → Supabase → painel React. A conexão local está configurada e a leitura pelo painel foi validada. S02 a S09 usam simulações locais. O painel oferece filtros, recomendações, favoritos locais e listas compartilhadas no Supabase. |
| O que cada parte faz | Edge: leitura simulada e decisão local (LEDs); Supabase: guarda o histórico da sala IoT e as listas compartilhadas; React/Web: exibição, filtros, rankings e CRUD de listas pela API.
| Repositório | https://github.com/JeckyCossio/study-comfort (público) |
| Indicador de impacto | Novas entrevistas com os usuários da plataforma, perguntando quanto tempo provavelmente foi economizado através do uso da plataforma, e quantas vezes por seman é usado a plataforma pelo usuário |
| ODS conectados | ODS 4: Educação de Qualidade (principal: melhores condições para estudar); ODS 9: Indústria, Inovação e Infraestrutura (apoio: IoT aplicada à infraestrutura do campus); possivelmente ODS 11: Cidades e Comunidades Sustentáveis (uso eficiente de espaços coletivos). |

### Equipe

| Integrante | RM |
|---|---|
| Daniel Roberto | 571746 |
| Felipe Pedrol | 569631 |
| Jecky Cossio | 572226 |
| Felipi Godoy | 573741 |

O StudyComfort ajuda estudantes a escolher uma sala considerando ocupação, temperatura, iluminação e ruído. O ESP32 representa a sala S01, com três mesas de três lugares, decide o estado no próprio dispositivo e sinaliza a condição por LEDs. A proposta se relaciona aos ODS 4, 9 e 11.

Todas as entradas são simuladas. O potenciômetro representa ruído em dB, sem microfone; a luminosidade é estimada pelo LDR. O protótipo não monitora uma sala física nem reserva assentos.

## Executar

1. Abra o [projeto final no Wokwi](https://wokwi.com/projects/480352166150085633).
2. Clique em **Start the simulation**. Não é necessário instalar programas ou conectar uma placa.
3. Acompanhe o Serial Monitor. O cenário inicial é 23 °C, aproximadamente 500 lux, 42,2 dB simulados e 0 de 9 lugares ocupados: estado adequado, LED verde.
4. Aguarde a conexão com `Wokwi-GUEST` e a mensagem `[HTTP] S01 publicada com sucesso (201).` para confirmar o envio ao Supabase.
5. Abra o [dashboard](https://gs-web-front-open-source-consulting.vercel.app/) e use **Preencher para mim** na entrada demonstrativa, quando disponível. Confira os indicadores da S01 e o horário da leitura.

Mantenha a simulação aberta e em execução. A S01 usa os dados do Wokwi; as demais salas usam dados demonstrativos. Execute apenas uma cópia da simulação por vez para não misturar os envios da mesma sala.

## Testar o circuito

Deixe temperatura em 23 °C, luz em aproximadamente 500 lux e ocupação baixa para testar somente o ruído.

| Teste | Como fazer | Resultado esperado |
| --- | --- | --- |
| Adequado | Potenciômetro em aproximadamente 25% | Cerca de 42,2 dB; LED verde |
| Atenção | Potenciômetro em 50% | Cerca de 55 dB; LED amarelo |
| Crítico | Potenciômetro no máximo | Cerca de 80 dB; LED vermelho contínuo |
| Falha de leitura | Ajustar o LDR para 100.000 lux | Neste extremo, a conversão gera leitura inválida; `sensor_error` e vermelho piscando |
| Recuperação | Restaurar LDR para aproximadamente 500 lux e potenciômetro para 25% | Retorno ao verde, com os demais indicadores adequados |
| Ocupação | Pressionar **Mesa 1 +** uma vez | Mesas `[1,0,0]`, 1 pessoa e 8 vagas |
| Limites dos botões | Tentar sair de mesa vazia ou entrar em mesa com 3 pessoas | Operação recusada no Serial; contagem permanece entre 0 e 3 |

Os botões `+` e `-` controlam cada mesa. Os atalhos de teclado são `1/2` para a mesa 1, `3/4` para a mesa 2 e `5/6` para a mesa 3. Ao reiniciar, a ocupação volta a zero.

Os sensores são lidos a cada 2,5 segundos de simulação, com média móvel de até cinco leituras. Aguarde o valor estabilizar no Serial após cada mudança. O envio HTTPS é síncrono e pode atrasar a atualização visual; não há garantia de estabilização em um tempo fixo de relógio.

## Entender os LEDs

| LED | Significado |
| --- | --- |
| Verde | Todos os indicadores adequados |
| Amarelo | Pelo menos um indicador em atenção, sem nenhum crítico |
| Vermelho contínuo | Pelo menos um indicador crítico |
| Vermelho piscando | Leitura inválida; verde e amarelo ficam apagados |

O estado geral acompanha a pior condição. Os limites são critérios didáticos adotados pelo grupo, sujeitos à validação em ambiente real.

| Indicador | Adequado | Atenção | Crítico |
| --- | --- | --- | --- |
| Temperatura | 21 a 25 °C | 18 a menos de 21 °C; acima de 25 até 26 °C | Abaixo de 18 ou acima de 26 °C |
| Luz estimada | 300 a 2.000 lux | 200 a menos de 300; acima de 2.000 até 3.000 lux | Abaixo de 200 ou acima de 3.000 lux |
| Ruído simulado | Até 45 dB | Acima de 45 até 65 dB | Acima de 65 dB |
| Ocupação | Até 50% | Acima de 50% até 90% | Acima de 90%; nesta sala, 9 de 9 lugares |

Falha de Wi-Fi ou publicação é informada no Serial e não aciona o vermelho por si só. A decisão local continua independente da conexão. Os alertas são visuais, por LEDs.

## Componentes e pinos

| Componente | Conexão ao ESP32 |
| --- | --- |
| DHT22 | SDA no GPIO 27; VCC em 3V3; GND comum |
| Módulo LDR | AO no GPIO 35; VCC em 3V3; GND comum |
| Potenciômetro de ruído | SIG no GPIO 34; VCC em 3V3; GND comum |
| Entradas das mesas 1, 2 e 3 | GPIOs 13, 16 e 18 |
| Saídas das mesas 1, 2 e 3 | GPIOs 14, 17 e 19 |
| LEDs verde, amarelo e vermelho | GPIOs 21, 22 e 23, com resistor de 220 Ω em cada LED |

Os botões usam `INPUT_PULLUP` e fecham para GND. A umidade do DHT22 não participa da classificação. O [diagram.json](wokwi/diagram.json) contém todas as ligações.

## Replicar em outro projeto Wokwi

1. Crie uma cópia do projeto final ou abra um projeto novo de ESP32 no Wokwi.
2. Substitua integralmente `sketch.ino`, `diagram.json` e `libraries.txt` pelos arquivos da pasta [wokwi](wokwi/).
3. Confira as bibliotecas **DHT sensor library for ESPx** e **ArduinoJson**, listadas em `libraries.txt`.
4. Inicie a simulação e repita os testes da tabela acima.

A configuração de demonstração já está no firmware. Para testar apenas o circuito, deixe `SUPABASE_URL` e `SUPABASE_PUBLISHABLE_KEY` vazios: as leituras e os LEDs continuam funcionando, sem envio à nuvem.

Para usar outro Supabase, configure a URL e a chave publicável do seu projeto e prepare o esquema e as permissões compatíveis com o POST descrito na [documentação técnica](documentacao/README.md). Os SQL em `apoio/sql/legado` pertencem à versão anterior e não recriam sozinhos o backend final.

## Caminho dos dados

**ESP32 no Wokwi → HTTPS POST → Supabase → dashboard StudyComfort.**

O firmware envia a leitura atual para `public.room_telemetry_history`, com `room_id = S01`, em intervalo mínimo de 10 segundos de simulação. HTTP 201 confirma a inserção. Com leitura inválida, os valores ambientais são enviados como `null` e `device_status` como `sensor_error`.

O dashboard consulta diretamente a leitura mais recente da S01 em `room_telemetry_history`, ordenando por `measured_at` e `measurement_id` decrescentes, com limite de um registro. Consulte os detalhes em [documentacao/README.md](documentacao/README.md).

O código público utiliza somente a chave publicável do ambiente demonstrativo. Não inclua chaves `secret` ou `service_role`. O uso de `setInsecure()` faz parte desta demonstração; uma instalação real exige validação do certificado e autenticação adequada do dispositivo.

## Organização

```text
edge/
├── README.md
├── .gitignore
├── entrega/
│   └── LINKS_E_EQUIPE.txt
├── wokwi/
│   ├── sketch.ino
│   ├── diagram.json
│   └── libraries.txt
├── video/
│   ├── README.md
│   └── video_edge.mp4
├── documentacao/
│   ├── README.md
│   └── VERIFICACAO.md
└── apoio/
    ├── sql/legado/
    └── roteiro/
```

## Entregar

Um integrante envia **o link do Wokwi e o link do YouTube**, com os nomes de toda a equipe, na tarefa do Teams ou no Forms indicado pelo professor. O texto está pronto em [LINKS_E_EQUIPE.txt](entrega/LINKS_E_EQUIPE.txt).

O edital de Edge não exige ZIP, relatório PDF ou link administrativo do Supabase. Esta pasta reúne o código, as instruções e os materiais de apoio do projeto.
