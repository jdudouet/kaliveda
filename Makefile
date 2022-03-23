#Makefile for KaliVeda web site/documentation

# set this for links to class doc in users guide & link to release notes on main page
export CLASS_DOC_BRANCH=1.12
# set this for latest version
export LATEST_RELEASE=1.12/05
# for git version in kaliveda splash on main page
export LATEST_GIT_RELEASE=1.12.05
# for date and time in kaliveda splash on main page
export DATE_TODAY=$(shell date +%F)
export TIME_NOW=$(shell date +%H:%M:%S)

###########################################
#                                         #
#   DO NOT CHANGE ANY OF THE FOLLOWING    #
#                                         #
###########################################

prefix = $(HOME)/kaliveda_web


WEBSITE_URL = http://indra.in2p3.fr/kaliveda
WEBSITE_ROOT_DIR = htdocs/kaliveda
WEBSITE_UPLOAD = indra@indra.in2p3.fr:$(WEBSITE_ROOT_DIR)

.PHONY : clean current default upload_current update_all upload_all update_and_upload_all

lyxdocdirs=$(shell ls $(prefix)/KaliVedaDoc | grep Doc)
		
main_site:
	@echo "Updating main website docs"
	@-mkdir -p $(prefix)/KaliVedaDoc
	@cd LyXGenDoc && $(MAKE) clean
	@cd LyXGenDoc && $(MAKE)
	@cd LyXGenDoc && $(MAKE) install PREFIX=$(prefix)/KaliVedaDoc
	@cat html/header.html userguide_TOC.html html/header_bot.html html/other_soft.html html/tail.html > $(prefix)/KaliVedaDoc/other_soft.html
	@cat html/header.html userguide_TOC.html html/header_bot.html html/prereq.html html/tail.html > $(prefix)/KaliVedaDoc/prereq.html
	@cat html/header.html userguide_TOC.html html/header_bot.html html/appli.html html/tail.html > $(prefix)/KaliVedaDoc/appli.html
	@cat html/header.html userguide_TOC.html html/header_bot.html html/build.html html/tail.html > $(prefix)/KaliVedaDoc/build.html
	@cat html/header.html userguide_TOC.html html/header_bot.html html/docker.html html/tail.html > $(prefix)/KaliVedaDoc/docker.html
	@cat html/header.html userguide_TOC.html html/header_bot.html html/download.html html/tail.html > $(prefix)/KaliVedaDoc/download.html
	@sed "s/CLASS_DOC_BRANCH/$(CLASS_DOC_BRANCH)/g" html/index.html > tmpfile.txt
	@sed -i "s|LATEST_RELEASE|$(LATEST_RELEASE)|g" tmpfile.txt
	@sed -i "s|LATEST_GIT_RELEASE|$(LATEST_GIT_RELEASE)|g" tmpfile.txt
	@sed -i "s|DATE_TODAY|$(DATE_TODAY)|g" tmpfile.txt
	@sed -i "s|TIME_NOW|$(TIME_NOW)|g" tmpfile.txt
	@cat html/header.html userguide_TOC.html html/header_bot.html tmpfile.txt html/tail.html > $(prefix)/KaliVedaDoc/index.html
	@cp -r css $(prefix)/KaliVedaDoc/
	@cp -r js $(prefix)/KaliVedaDoc/
	@cp -r images $(prefix)/KaliVedaDoc/
	
	
users_guide:
	@echo "Updating users guide"
	@cd usersguide && $(MAKE) && $(MAKE) install PREFIX=$(prefix)/KaliVedaDoc

update_all: users_guide main_site
clean:
	@cd LyXGenDoc && $(MAKE) clean
	@cd usersguide && $(MAKE) clean
	@rm -f upload.batch userguide_TOC.html
        
distclean: clean
	-rm -rf KaliVedaDoc
 
