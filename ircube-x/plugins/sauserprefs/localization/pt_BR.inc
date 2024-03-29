<?php
/* Author: Alexandre Correa */

$labels = [];
$labels['sauserprefs'] = 'Spam';
$labels['sauserprefssettings'] = 'Configurações Spam';
$labels['spamgeneralsettings'] = 'Configurações Gerais';
$labels['spamthres'] = 'Pontuação Inicial';
$labels['spamsubject'] = 'Marca do Assunto';
$labels['spamsubjectblank'] = '(deixe em branco para nenhum)';
$labels['spamlang'] = 'Linguagem das mensagens';
$labels['enabled'] = 'Ligado';
$labels['disabled'] = 'Desligado';
$labels['headers'] = 'Cabeçalho';
$labels['foldheaders'] = 'Permitir multiplas linhas no cabeçalho';
$labels['spamlevelstars'] = 'Usar \'Nivel de Spam\' no cabeçalho';
$labels['spamlevelchar'] = 'Marcador de nível';
$labels['spamtests'] = 'Testes de spam ON-LINE';
$labels['userazor1'] = 'Usar Lista Razor V1';
$labels['userazor2'] = 'Usar Lista Razor V2';
$labels['usepyzor'] = 'Usar Lista Pyzor';
$labels['usebayes'] = 'Usar classificação \'Bayesian\'';
$labels['usedcc'] = 'Usar DCC';
$labels['skiprblchecks'] = 'Usar Lista-Negra em tempo real';
$labels['spamreportsettings'] = 'Configurações de Notificação';
$labels['spamreport0'] = 'Incluir notas nos cabeçalhos da mensagem original';
$labels['spamreport1'] = 'Incluir mensagem como anexo MIME';
$labels['spamreport2'] = 'Incluir mensagem como texto plano';
$labels['spamaddressrules'] = 'Regras de Endereços';
$labels['welcomelist_from'] = 'Aceitar email de';
$labels['blocklist_from'] = 'Rejeitar email de';
$labels['welcomelist_to'] = 'Aceitar email enviado para';
$labels['addrule'] = 'Adicionar regra';
$labels['rule'] = 'Regra';
$labels['importfromaddressbook'] = 'Importar endereço do Livro de Endereços';
$labels['deleteall'] = 'Apagar tudo';
$labels['autoscore'] = 'Auto';
$labels['defaultscore'] = 'Pontuação Padrão';
$labels['otherscore'] = 'Outro (%s)';
$labels['saupusedefault'] = 'Voltar configuração padrão';
$labels['langoptions'] = 'Opções de linguagem';
$labels['bayes'] = 'Configuração \'Bayes\'';
$labels['purgingbayes'] = 'Apagar dados da \'Bayes\'';
$labels['purgebayes'] = 'Apagar dados pessoais da \'Bayes\'';
$labels['bayesautolearn'] = 'Usar auto-aprendizagem da \'Bayes\'';
$labels['bayesautooptions'] = 'Opções de auto-aprendizagem';
$labels['bayesnonspam'] = 'Pontuação de Não-Spam';
$labels['bayesspam'] = 'Potuação de Spam';
$labels['bayesrules'] = 'Usar regras \'Baysian\'';
$labels['help'] = 'Ajuda';

$messages = [];
$messages['sauserprefchanged'] = 'Configurações de SPAM salvas';
$messages['sauserpreffailed'] = 'Erro: Não foi possível alterar configurações de SPAM';
$messages['spamthresexp'] = 'Qualquer valor acima da pontuação será marcado como SPAM. Aumentando a pontuação pode aumentar a quantidade de SPAM não detectado, mas evita que uma mensagem verdadeira seja marcada como SPAM.';
$messages['spamlangexp'] = 'Selecione todos os idiomas que você espera receber mensagens. Qualquer mensagem recebida de idiomas não selecionados são marcadas como SPAM.';
$messages['headersexp'] = 'SpamAssassin adiciona cabeçalhos nas mensagens processadas com informações sobre os resultados obtidos. Utilize as opções abaixo para configurar:';
$messages['spamtestssexp'] = 'Algumas verificações fazem consultas on-line afim de ajudar a detectar SPAM. Selecione as opções que deseja utilizar:';
$messages['spamreport'] = 'Quando uma mensagem é identificada como SPAM, uma notificação de todos os testes e pontuações obtidas é criada...';
$messages['autowelcomelist'] = 'Endereços de email cadastrados em sua agenda são automaticamente adicionados na lista de "Aceitar email de".';
$messages['welcomelistexp'] = 'Asterisco pode ser utilizado como curinga para uma maior flexibilidade. Exemplo: *@exemplo.com.br ou *.exemplo.com.br';
$messages['spamaddressexists'] = 'Uma regra já existe para este endereço';
$messages['spamenteraddress'] = 'Digite um endereço para adicionar';
$messages['spamaddresserror'] = 'Erro: Endereço inválido';
$messages['spamaddressdelete'] = 'Tem certeza que deseja excluir esta regra ?';
$messages['spamaddressdeleteall'] = 'Tem certeza que deseja excluir TODAS as regras ?';
$messages['noaddressrules'] = 'Regras de endereço não encontradas.';
$messages['importingaddresses'] = 'Importando endereços...';
$messages['usedefaultconfirm'] = 'Tem certeza que deseja restaurar para os padrões iniciais? Isto apaga todas as configurações, incluindo regras de endereços';
$messages['purgebayesexp'] = 'Apagar configurações pessoais \'Bayes\' coletadas em seu email';
$messages['bayesnonspamexp'] = 'Mensagens que tenham pontuação abaixo deste nível são utilizadas para aprimorar o sistema de detecção de menagens NÃO-SPAM.';
$messages['bayesspamexp'] = 'Mensagens que tenham pontuação acima deste nível são utilizadas para aprimorar o sistema de detecção de mensagens SPAM.';
$messages['bayesautohelp'] = 'Quando a auto-aprendizagem \'Bayes\' está ativada, o sistema utiliza as mensagens marcadas como SPAM/NÃO-SPAM para aprimorar o sistema de detecção.';
$messages['bayeshelp'] = 'O sistema \'Bayes\' tenta identificar SPAM analisando as palavras ou textos curtos sequenciais que, em comum, são encontrados nas mensagems SPAM/NÃO-SPAM.';
$messages['rblhelp'] = 'Endereço do servidor encontrado nos atalhos na mensagem são verificadas nas listas-negra ou listas de spam.';
$messages['dcchelp'] = 'O DCC é um sistema composto por servidores que coletam e contabilizam as assinaturas digitais de milhões de mensagens. Estes contadores podem ser utilizados para detectar spam.';
$messages['pyzhelp'] = 'Pyzor é um sistema colaborativo e interligado para detectar e bloquear mensagens identificadas pelo seu indice.';
$messages['raz2help'] = '\'Vipuls Razor\' é um sistema baseado nas notificações feitas por vários usuários. A detecção é feita com base na assinatura digital da mensagem.';
$messages['raz1help'] = '\'Vipuls Razor\' é um sistema baseado nas notificações feitas por vários usuários. A detecção é feita com base na assinatura digital da mensagem.';
$messages['levelhelp'] = 'O cabeçalho \'Spam Level\' pode ser adicionado na mensagem repetindo o \'Marcado de nível\' de acordo com a pontuação obtida.';
$messages['foldhelp'] = 'Por padrão, os cabeçalhos adicionados são separados por duplo espaço. Evitando uma linha muito longa, quebrando-a em várias linhas menores.';
$messages['bayesruleshlp'] = 'Se usar as regras \'naive-Bayes-style\' como classificadores. Permite que desligue as regras mas, possibilitando deixar auto-aprendizagem automatica ou manual.';
$messages['purgebayesconfirm'] = 'Tem certeza que deseja excluir os dados da \'Bayes\' ?';
