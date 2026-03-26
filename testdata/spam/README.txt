mailfilter campaign test set
===========================

Zweck
-----
Dieses Set erzeugt bewusst mehrere kleine, wiederkehrende Spam-Kampagnen, damit
campaign signatures, domain candidates, score candidates und recommended rules
sichtbar werden.

Inhalt
------
spam/campaign_alpha/
  inflink.tipsy.chat + Mailjet + price/urgency/ultrasonic

spam/campaign_beta/
  deals.example-store + Hornetsecurity/Antispameurope + smart-watch promo

spam/campaign_gamma/
  paypai-secure-check.example + paypal-lookalike phishing

ham/legit_bursts/
  legitime, wiederkehrende Ham-Mails (Pollin, PayPal, GitHub, Amazon, FixYourAudio)

Empfohlener Import
------------------
Spam:
  ./mailfilter-header-import.pl \
    --db /var/spool/filter/mailheader-campaign-test.sqlite3 \
    --schema-from /var/spool/filter/mailheader.log.sqlite3 \
    --input spam \
    --decision deny \
    --id-prefix campdeny \
    --reset

Ham:
  ./mailfilter-header-import.pl \
    --db /var/spool/filter/mailheader-campaign-test.sqlite3 \
    --input ham \
    --decision pass \
    --id-prefix campham

Rulegen:
  ./mailfilter-rulegen.sh \
    --db /var/spool/filter/mailheader-campaign-test.sqlite3 \
    --mailfilterrc /etc/mailfilter/.mailfilterrc \
    --out generated-campaign-candidates.conf \
    --highscore 100 \
    --min-deny-hits 2 \
    --max-pass-hits 0 \
    --min-phrase-size 2 \
    --max-phrase-size 3

Was sichtbar werden sollte
--------------------------
- campaign signatures für Alpha/Beta/Gamma
- domain candidates für inflink.tipsy.chat, deals.example-store, paypai-secure-check.example
- score candidates für price/promo/urgency
- protect/allow suppression bei Ham-Beispielen
