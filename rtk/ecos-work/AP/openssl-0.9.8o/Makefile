include $(ECOS_WORK_DIR)/.config

#------------------------------------------------------------------------------

INCLUDES= -I./include -I./include/openssl -I./

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

EXTRA_CFLAGS +=  -DOPENSSL_NO_KRB5 $(INCLUDES)
#EXTRA_CFLAGS +=  -DOPENSSL_FIPS  -DRSDK_BUILT -DOPENSSL_NO_SPEED -DOPENSSL_THREADS -D_REENTRANT \
#		-DDSO_DLFCN -DHAVE_DLFCN_H -DOPENSSL_NO_KRB5 -DB_ENDIAN -DTERMIO 
		
OPENSSL_AES = 1
OPENSSL_ECDH = 
OPENSSL_ECDSA = 
OPENSSL_IDEA = 
OPENSSL_RC5 = 
OPENSSL_RSA = 1
OPENSSL_MD2 = 
OPENSSL_MD4 = 1
OPENSSL_MD5 = 1
OPENSSL_SHA = 
OPENSSL_SHA1 = 1
OPENSSL_RMD160 = 1
OPENSSL_DES = 1
OPENSSL_RC2 = 
OPENSSL_BLOWFISH = 1
OPENSSL_CAST = 
OPENSSL_MDC2 = 
OPENSSL_DSA = 1
OPENSSL_DH = 1
OPENSSL_BIO = 1
OPENSSL_BUFFER = 1
OPENSSL_COMP = 
OPENSSL_ERR = 
OPENSSL_SSL = 1
ifeq ($(OPENSSL_SSL),1)
OPENSSL_MD5 = 1
OPENSSL_RC4 = 
OPENSSL_RC2 =  
OPENSSL_IDEA = 
OPENSSL_DES = 1
OPENSSL_SHA = 1
OPENSSL_COMP = 
endif

ifneq ($(OPENSSL_AES),1)
EXTRA_CFLAGS += -DOPENSSL_NO_AES
EXTRA_CFLAGS += -DNO_AES
endif

ifneq ($(OPENSSL_ECDH),1)
EXTRA_CFLAGS += -DOPENSSL_NO_ECDH
EXTRA_CFLAGS += -DNO_ECDH
endif

ifneq ($(OPENSSL_ECDSA),1)
EXTRA_CFLAGS += -DOPENSSL_NO_ECDSA
EXTRA_CFLAGS += -DNO_ECDSA
endif

ifneq ($(OPENSSL_IDEA),1)
EXTRA_CFLAGS += -DOPENSSL_NO_IDEA
EXTRA_CFLAGS += -DNO_IDEA
endif

ifneq ($(OPENSSL_RC5),1)
EXTRA_CFLAGS += -DOPENSSL_NO_RC5
EXTRA_CFLAGS += -DNO_RC5
endif

ifneq ($(OPENSSL_RSA),1)
EXTRA_CFLAGS += -DOPENSSL_NO_RSA
EXTRA_CFLAGS += -DNO_RSA
endif

ifneq ($(OPENSSL_MD2),1)
EXTRA_CFLAGS += -DOPENSSL_NO_MD2
EXTRA_CFLAGS += -DNO_MD2
endif

ifneq ($(OPENSSL_MD4),1)
EXTRA_CFLAGS += -DOPENSSL_NO_MD4
EXTRA_CFLAGS += -DNO_MD4
endif

ifneq ($(OPENSSL_MD5),1)
EXTRA_CFLAGS += -DOPENSSL_NO_MD5
EXTRA_CFLAGS += -DNO_MD5
endif

ifneq ($(OPENSSL_SHA),1)
EXTRA_CFLAGS += -DOPENSSL_NO_SHA
EXTRA_CFLAGS += -DNO_SHA
endif

ifneq ($(OPENSSL_SHA1),1)
EXTRA_CFLAGS += -DOPENSSL_NO_SHA1
EXTRA_CFLAGS += -DNO_SHA1
endif

ifneq ($(OPENSSL_RMD160),1)
EXTRA_CFLAGS += -DOPENSSL_NO_RMD160
EXTRA_CFLAGS += -DNO_RMD160
endif

ifneq ($(OPENSSL_DES),1)
EXTRA_CFLAGS += -DOPENSSL_NO_DES
EXTRA_CFLAGS += -DNO_DES
endif

ifneq ($(OPENSSL_RC4),1)
EXTRA_CFLAGS += -DOPENSSL_NO_RC4
EXTRA_CFLAGS += -DNO_RC4
endif

ifneq ($(OPENSSL_RC2),1)
EXTRA_CFLAGS += -DOPENSSL_NO_RC2
EXTRA_CFLAGS += -DNO_RC2
endif

ifneq ($(OPENSSL_BLOWFISH),1)
EXTRA_CFLAGS += -DOPENSSL_NO_BLOWFISH
EXTRA_CFLAGS += -DNO_BLOWFISH
endif

ifneq ($(OPENSSL_CAST),1)
EXTRA_CFLAGS += -DOPENSSL_NO_CAST
EXTRA_CFLAGS += -DNO_CAST
endif

ifneq ($(OPENSSL_MDC2),1)
EXTRA_CFLAGS += -DOPENSSL_NO_MDC2
EXTRA_CFLAGS += -DNO_MDC2
endif

ifneq ($(OPENSSL_DSA),1)
EXTRA_CFLAGS += -DOPENSSL_NO_DSA
EXTRA_CFLAGS += -DNO_DSA
endif

ifneq ($(OPENSSL_DH),1)
EXTRA_CFLAGS += -DOPENSSL_NO_DH
EXTRA_CFLAGS += -DNO_DH
endif

ifneq ($(OPENSSL_BIO),1)
EXTRA_CFLAGS += -DOPENSSL_NO_BIO
EXTRA_CFLAGS += -DNO_BIO
endif

ifneq ($(OPENSSL_BUFFER),1)
EXTRA_CFLAGS += -DOPENSSL_NO_BUFFER
EXTRA_CFLAGS += -DNO_BUFFER
endif

ifneq ($(OPENSSL_COMP),1)
EXTRA_CFLAGS += -DOPENSSL_NO_COMP
EXTRA_CFLAGS += -DNO_COMP
endif

ifneq ($(OPENSSL_SSL),1)
EXTRA_CFLAGS += -DOPENSSL_NO_SSL
EXTRA_CFLAGS += -DNO_SSL
endif

ifneq ($(OPENSSL_ERR),1)
EXTRA_CFLAGS += -DOPENSSL_NO_ERR
EXTRA_CFLAGS += -DNO_ERR
endif
OPENSSL_EC = 1
ifneq ($(OPENSSL_EC),1)
EXTRA_CFLAGS += -DOPENSSL_NO_EC
EXTRA_CFLAGS += -DNO_EC
endif



#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S
SOURCES = crypto/asn1/a_object.c  \
        crypto/asn1/a_bitstr.c  \
        crypto/asn1/a_utctm.c   \
        crypto/asn1/a_gentm.c   \
        crypto/asn1/a_time.c    \
        crypto/asn1/a_int.c     \
        crypto/asn1/a_octet.c   \
        crypto/asn1/a_print.c   \
        crypto/asn1/a_type.c    \
        crypto/asn1/a_set.c     \
        crypto/asn1/a_dup.c     \
        crypto/asn1/a_d2i_fp.c  \
        crypto/asn1/a_i2d_fp.c  \
        crypto/asn1/a_enum.c    \
        crypto/asn1/a_utf8.c    \
        crypto/asn1/a_sign.c    \
        crypto/asn1/a_digest.c  \
        crypto/asn1/a_verify.c  \
        crypto/asn1/a_mbstr.c   \
        crypto/asn1/a_strex.c   \
        crypto/asn1/x_algor.c   \
        crypto/asn1/x_val.c     \
        crypto/asn1/x_pubkey.c  \
        crypto/asn1/x_sig.c     \
        crypto/asn1/x_req.c     \
        crypto/asn1/x_attrib.c  \
        crypto/asn1/x_name.c    \
        crypto/asn1/x_x509.c    \
        crypto/asn1/x_x509a.c   \
        crypto/asn1/x_crl.c     \
        crypto/asn1/x_info.c    \
        crypto/asn1/x_spki.c    \
        crypto/asn1/nsseq.c     \
        crypto/asn1/d2i_pu.c    \
        crypto/asn1/d2i_pr.c    \
        crypto/asn1/i2d_pu.c    \
        crypto/asn1/i2d_pr.c    \
        crypto/asn1/f_int.c     \
        crypto/asn1/f_string.c  \
        crypto/asn1/n_pkey.c    \
        crypto/asn1/f_enum.c    \
        crypto/asn1/a_hdr.c     \
        crypto/asn1/x_pkey.c    \
        crypto/asn1/a_bool.c    \
        crypto/asn1/x_exten.c   \
        crypto/asn1/x_crl.c 	\
        crypto/asn1/x_bignum.c	\
        crypto/asn1/x_long.c	\
        crypto/asn1/x_x509a.c	\
        crypto/asn1/asn1_par.c  \
        crypto/asn1/asn1_lib.c  \
        crypto/asn1/asn1_err.c  \
        crypto/asn1/asn1_gen.c	\
        crypto/asn1/a_meth.c    \
        crypto/asn1/a_bytes.c   \
        crypto/asn1/a_strnid.c  \
        crypto/asn1/evp_asn1.c  \
        crypto/asn1/asn_pack.c  \
        crypto/asn1/p5_pbe.c    \
        crypto/asn1/p5_pbev2.c  \
        crypto/asn1/p8_pkey.c   \
        crypto/asn1/tasn_dec.c  \
        crypto/asn1/tasn_fre.c	\
        crypto/asn1/tasn_typ.c	\
        crypto/asn1/tasn_utl.c	\
        crypto/asn1/tasn_new.c	\
        crypto/asn1/tasn_enc.c	\
        crypto/asn1/a_time.c	\
         crypto/asn1/t_req.c     \
        crypto/asn1/t_x509.c    \
        crypto/asn1/t_x509a.c   \
        crypto/asn1/t_crl.c     \
        crypto/asn1/t_pkey.c    \
        crypto/asn1/t_spki.c    \
        crypto/asn1/t_bitst.c   \
        crypto/x509/x_all.c     \
        crypto/stack/stack.c    \
        crypto/bn/bn_add.c      \
        crypto/bn/bn_div.c      \
        crypto/bn/bn_exp.c      \
        crypto/bn/bn_lib.c      \
        crypto/bn/bn_ctx.c      \
        crypto/bn/bn_mul.c      \
        crypto/bn/bn_mod.c		\
        crypto/bn/bn_print.c    \
        crypto/bn/bn_rand.c     \
        crypto/bn/bn_shift.c    \
        crypto/bn/bn_word.c     \
        crypto/bn/bn_blind.c    \
        crypto/bn/bn_gcd.c      \
        crypto/bn/bn_prime.c    \
        crypto/bn/bn_err.c      \
        crypto/bn/bn_sqr.c      \
        crypto/bn/bn_asm.c      \
        crypto/bn/bn_recp.c     \
        crypto/bn/bn_mont.c     \
        crypto/bn/bn_mpi.c      \
        crypto/bn/bn_exp2.c     \
        crypto/bn/bn_gf2m.c		\
        crypto/bn/bn_kron.c		\
        crypto/bn/bn_sqrt.c		\
        crypto/bn/bn_nist.c		\
        crypto/dso/dso_err.c	\
        crypto/evp/encode.c     \
        crypto/evp/digest.c     \
        crypto/evp/evp_enc.c    \
        crypto/evp/evp_key.c    \
        crypto/evp/e_des.c      \
        crypto/evp/e_bf.c       \
        crypto/evp/e_idea.c     \
        crypto/evp/e_des3.c     \
        crypto/evp/e_rc4.c      \
        crypto/evp/e_aes.c    	\
        crypto/evp/names.c      \
        crypto/evp/e_xcbc_d.c   \
        crypto/evp/e_rc2.c      \
        crypto/evp/e_cast.c     \
        crypto/evp/e_rc5.c      \
        crypto/evp/m_null.c     \
        crypto/evp/m_md2.c      \
        crypto/evp/m_md4.c      \
        crypto/evp/m_md5.c      \
        crypto/evp/m_sha.c      \
        crypto/evp/m_sha1.c     \
        crypto/evp/m_dss.c      \
        crypto/evp/m_dss1.c     \
        crypto/evp/m_mdc2.c     \
        crypto/evp/p_open.c     \
        crypto/evp/p_seal.c     \
        crypto/evp/p_sign.c     \
        crypto/evp/p_verify.c   \
        crypto/evp/p_lib.c      \
        crypto/evp/p_enc.c      \
        crypto/evp/p_dec.c      \
        crypto/evp/bio_md.c     \
        crypto/evp/bio_b64.c    \
        crypto/evp/bio_enc.c    \
        crypto/evp/evp_err.c    \
        crypto/evp/e_null.c     \
        crypto/evp/c_all.c      \
        crypto/evp/c_allc.c     \
        crypto/evp/c_alld.c     \
        crypto/evp/evp_lib.c    \
        crypto/evp/bio_ok.c     \
        crypto/evp/evp_pkey.c   \
        crypto/evp/evp_pbe.c    \
        crypto/evp/p5_crpt.c    \
        crypto/evp/p5_crpt2.c   \
        crypto/evp/enc_min.c	\
        crypto/evp/m_ecdsa.c	\
        crypto/evp/e_seed.c		\
        crypto/evp/m_ripemd.c   \
        crypto/conf/conf_lib.c  \
        crypto/conf/conf_api.c  \
        crypto/conf/conf_def.c  \
        crypto/conf/conf_mod.c	\
        crypto/hmac/hmac.c      \
        crypto/lhash/lhash.c    \
        crypto/lhash/lh_stats.c \
        crypto/objects/o_names.c \
        crypto/objects/obj_dat.c \
        crypto/objects/obj_lib.c \
        crypto/ocsp/ocsp_asn.c	\
        crypto/pem/pem_sign.c    \
        crypto/pem/pem_seal.c   \
        crypto/pem/pem_info.c   \
        crypto/pem/pem_lib.c    \
        crypto/pem/pem_all.c    \
        crypto/pem/pem_err.c    \
        crypto/pem/pem_oth.c	\
        crypto/pem/pem_xaux.c	\
        crypto/pem/pem_x509.c	\
        crypto/pem/pem_pkey.c	\
        crypto/pkcs12/p12_add.c  \
        crypto/pkcs12/p12_attr.c \
        crypto/pkcs12/p12_crpt.c \
        crypto/pkcs12/p12_crt.c  \
        crypto/pkcs12/p12_decr.c \
        crypto/pkcs12/p12_init.c \
        crypto/pkcs12/p12_key.c  \
        crypto/pkcs12/p12_kiss.c \
        crypto/pkcs12/p12_mutl.c \
        crypto/pkcs12/p12_utl.c  \
        crypto/pkcs12/p12_npas.c \
        crypto/pkcs12/p12_p8d.c	\
        crypto/pkcs12/pk12err.c \
        crypto/rand/md_rand.c   \
        crypto/rand/randfile.c  \
        crypto/rand/rand_lib.c  \
        crypto/rand/rand_err.c  \
        crypto/rand/rand_egd.c  \
        crypto/rand/rand_ecos.c  \
        crypto/x509/x509_def.c  \
        crypto/x509/x509_d2.c   \
        crypto/x509/x509_r2x.c  \
        crypto/x509/x509_cmp.c  \
        crypto/x509/x509_obj.c  \
        crypto/x509/x509_req.c  \
        crypto/x509/x509spki.c  \
        crypto/x509/x509_vfy.c  \
        crypto/x509/x509_set.c  \
        crypto/x509/x509rset.c  \
        crypto/x509/x509_err.c  \
        crypto/x509/x509name.c  \
        crypto/x509/x509_v3.c   \
        crypto/x509/x509_ext.c  \
        crypto/x509/x509_att.c  \
        crypto/x509/x509type.c  \
        crypto/x509/x509_lu.c   \
        crypto/x509/x509_txt.c  \
        crypto/x509/x509_trs.c  \
        crypto/x509/by_file.c   \
        crypto/x509/by_dir.c    \
        crypto/x509/x509_vfy.c	\
        crypto/x509/x509_vpm.c	\
        crypto/x509v3/v3_bcons.c \
        crypto/x509v3/v3_bitst.c \
        crypto/x509v3/v3_conf.c  \
        crypto/x509v3/v3_extku.c \
        crypto/x509v3/v3_ia5.c  \
        crypto/x509v3/v3_lib.c  \
        crypto/x509v3/v3_prn.c  \
        crypto/x509v3/v3_utl.c  \
        crypto/x509v3/v3err.c   \
        crypto/x509v3/v3_genn.c \
        crypto/x509v3/v3_alt.c  \
        crypto/x509v3/v3_skey.c \
        crypto/x509v3/v3_akey.c \
        crypto/x509v3/v3_pku.c  \
        crypto/x509v3/v3_int.c  \
        crypto/x509v3/v3_enum.c \
        crypto/x509v3/v3_sxnet.c \
        crypto/x509v3/v3_cpols.c \
        crypto/x509v3/v3_crld.c  \
        crypto/x509v3/v3_purp.c \
        crypto/x509v3/v3_info.c \
        crypto/x509v3/v3_ocsp.c \
        crypto/x509v3/v3_pcia.c \
        crypto/x509v3/v3_akeya.c \
        crypto/x509v3/v3_pci.c	\
        crypto/x509v3/v3_pcons.c	\
        crypto/x509v3/pcy_cache.c	\
        crypto/x509v3/pcy_data.c	\
        crypto/x509v3/v3_ncons.c	\
        crypto/x509v3/v3_pmaps.c	\
        crypto/x509v3/v3_asid.c		\
        crypto/x509v3/v3_addr.c		\
        crypto/x509v3/pcy_tree.c	\
        crypto/x509v3/pcy_lib.c		\
        crypto/x509v3/pcy_node.c	\
        crypto/x509v3/pcy_map.c		\
        crypto/err/err_all.c    \
        crypto/err/err_prn.c    \
        crypto/err/err.c        \
        crypto/err/err_def.c    \
        crypto/err/err_str.c	\
        crypto/ui/ui_lib.c		\
        crypto/ui/ui_err.c		\
		crypto/ui/ui_openssl.c	\
        crypto/cryptlib.c       \
        crypto/mem.c            \
        crypto/mem_clr.c		\
        crypto/mem_dbg.c        \
        crypto/cversion.c       \
        crypto/ex_data.c        \
        crypto/cpt_err.c        \
        crypto/ebcdic.c         \
        crypto/dyn_lck.c         \
        crypto/o_time.c			\
        crypto/uid.c            
#        crypto/seed/seed.c	crypto/seed/seed_cbc.c	crypto/seed/seed_ofb.c	crypto/seed/seed_cfb.c	crypto/seed/seed_ecb.c  

ifeq ($(OPENSSL_EC),1)
SOURCES += crypto/ec/ec_err.c		\
        crypto/ec/ec_key.c		\
        crypto/ec/ec_lib.c		\
        crypto/ec/ec_asn1.c		\
        crypto/ec/ec_curve.c	\
        crypto/ec/ec_cvt.c		\
        crypto/ec/ec2_smpl.c	\
        crypto/ec/ec_mult.c		\
        crypto/ec/ecp_mont.c	\
        crypto/ec/ecp_smpl.c	\
        crypto/ec/ec2_mult.c	\
        crypto/ec/ec_cvt.c		\
        crypto/ec/ecp_nist.c
endif
ifeq ($(OPENSSL_AES),1)
SOURCES += crypto/aes/aes_cbc.c	\
        crypto/aes/aes_core.c \
        crypto/aes/aes_cfb.c \
        crypto/aes/aes_ecb.c \
        crypto/aes/aes_ofb.c
endif
ifeq ($(OPENSSL_ECDH),1)
SOURCES += crypto/ecdh/ech_err.c	\
      	crypto/ecdh/ech_key.c	\
      	crypto/ecdh/ech_ossl.c	\
      	crypto/ecdh/ech_lib.c	
endif
ifeq ($(OPENSSL_ECDSA),1)
SOURCES += crypto/ecdsa/ecs_sign.c	\
		crypto/ecdsa/ecs_vrf.c	\
		crypto/ecdsa/ecs_asn1.c	\
		crypto/ecdsa/ecs_err.c	\
		crypto/ecdsa/ecs_lib.c	\
		crypto/ecdsa/ecs_ossl.c	   
endif
ifeq ($(OPENSSL_IDEA),1)
SOURCES +=  crypto/idea/i_cbc.c     \
       crypto/idea/i_cfb64.c   \
       crypto/idea/i_ofb64.c   \
       crypto/idea/i_ecb.c     \
       crypto/idea/i_skey.c  
endif
ifeq ($(OPENSSL_RC5),1)
SOURCES += crypto/rc5/rc5_skey.c   \
      crypto/rc5/rc5_ecb.c    \
      crypto/rc5/rc5_enc.c    \
      crypto/rc5/rc5cfb64.c   \
      crypto/rc5/rc5ofb64.c  
endif
ifeq ($(OPENSSL_RSA),1)
SOURCES +=crypto/rsa/rsa_eay.c    \
	   crypto/rsa/rsa_chk.c    \
	   crypto/rsa/rsa_depr.c   \
       crypto/rsa/rsa_gen.c    \
       crypto/rsa/rsa_lib.c    \
       crypto/rsa/rsa_sign.c   \
       crypto/rsa/rsa_saos.c   \
       crypto/rsa/rsa_err.c    \
       crypto/rsa/rsa_pk1.c    \
       crypto/rsa/rsa_ssl.c    \
       crypto/rsa/rsa_none.c   \
       crypto/rsa/rsa_oaep.c   \
       crypto/rsa/rsa_null.c	\
       crypto/rsa/rsa_asn1.c	\
       crypto/rsa/rsa_eng.c	\
       crypto/rsa/rsa_x931.c 
endif
ifeq ($(OPENSSL_MD2),1)
SOURCES += crypto/md2/md2_dgst.c   \
       crypto/md2/md2_one.c  
endif
ifeq ($(OPENSSL_MD4),1)
SOURCES += crypto/md4/md4_dgst.c   \
       crypto/md4/md4_one.c 
endif
ifeq ($(OPENSSL_MD5),1)
SOURCES +=  crypto/md5/md5_dgst.c   \
       crypto/md5/md5_one.c  
endif
ifeq ($(OPENSSL_SHA),1)
SOURCES += crypto/sha/sha_dgst.c   \
       crypto/sha/sha_one.c   	\
       crypto/sha/sha256.c	\
       crypto/sha/sha512.c
endif
ifeq ($(OPENSSL_SHA1),1)
SOURCES += crypto/sha/sha1dgst.c   \
        crypto/sha/sha1_one.c 
endif
ifeq ($(OPENSSL_RMD160),1)
SOURCES += crypto/ripemd/rmd_dgst.c   \
        crypto/ripemd/rmd_one.c 
endif
ifeq ($(OPENSSL_DES),1)
SOURCES += crypto/des/set_key.c    \
       crypto/des/ecb_enc.c    \
       crypto/des/cbc_enc.c    \
       crypto/des/ecb3_enc.c   \
       crypto/des/cfb64enc.c   \
       crypto/des/cfb64ede.c   \
       crypto/des/cfb_enc.c    \
       crypto/des/ofb64ede.c   \
       crypto/des/enc_read.c   \
       crypto/des/enc_writ.c   \
       crypto/des/ofb64enc.c   \
       crypto/des/ofb_enc.c    \
       crypto/des/str2key.c    \
       crypto/des/pcbc_enc.c   \
       crypto/des/qud_cksm.c   \
       crypto/des/rand_key.c   \
       crypto/des/des_enc.c    \
       crypto/des/des_old.c    \
       crypto/des/fcrypt.c     \
       crypto/des/fcrypt_b.c   \
       crypto/des/xcbc_enc.c   \
       crypto/des/rpc_enc.c    \
       crypto/des/cbc_cksm.c   \
       crypto/des/ede_cbcm_enc.c 
endif
ifeq ($(OPENSSL_RC4),1)
SOURCES += crypto/rc4/rc4_skey.c   \
        crypto/rc4/rc4_enc.c
endif
ifeq ($(OPENSSL_RC2),1)
SOURCES +=  crypto/rc2/rc2_ecb.c    \
       crypto/rc2/rc2_skey.c   \
       crypto/rc2/rc2_cbc.c    \
       crypto/rc2/rc2cfb64.c   \
       crypto/rc2/rc2ofb64.c
endif
ifeq ($(OPENSSL_BLOWFISH),1)
SOURCES += crypto/bf/bf_skey.c     \
	      crypto/bf/bf_ecb.c      \
	      crypto/bf/bf_enc.c      \
	      crypto/bf/bf_cfb64.c    \
	      crypto/bf/bf_ofb64.c    
endif
ifeq ($(OPENSSL_CAST),1)
SOURCES += crypto/cast/c_skey.c    \
	      crypto/cast/c_ecb.c     \
	      crypto/cast/c_cfb64.c   \
	      crypto/cast/c_ofb64.c   \
	      crypto/cast/c_enc.c     
endif
ifeq ($(OPENSSL_MDC2),1)
SOURCES +=  crypto/mdc2/mdc2dgst.c  \
         crypto/mdc2/mdc2_one.c 
endif
ifeq ($(OPENSSL_DSA),1)
SOURCES += crypto/dsa/dsa_gen.c    \
	      crypto/dsa/dsa_key.c    \
	      crypto/dsa/dsa_lib.c    \
	      crypto/dsa/dsa_asn1.c   \
	      crypto/dsa/dsa_vrf.c    \
	      crypto/dsa/dsa_sign.c   \
	      crypto/dsa/dsa_err.c    \
	      crypto/dsa/dsa_ossl.c   \
	      crypto/dsa/dsa_utl.c
endif
ifeq ($(OPENSSL_DH),1)
SOURCES += crypto/dh/dh_gen.c      \
	       crypto/dh/dh_key.c      \
	       crypto/dh/dh_lib.c      \
	       crypto/dh/dh_check.c crypto/dh/dh_asn1.c	crypto/dh/dh_err.c
endif
 
ifeq ($(OPENSSL_BIO),1)
SOURCES += crypto/bio/bio_lib.c    \
          crypto/bio/bio_cb.c     \
          crypto/bio/bio_err.c    \
          crypto/bio/bss_mem.c    \
          crypto/bio/bss_null.c   \
          crypto/bio/bss_fd.c     \
          crypto/bio/bss_file.c   \
          crypto/bio/bss_sock.c   \
          crypto/bio/bss_conn.c   \
          crypto/bio/bf_null.c    \
          crypto/bio/bf_buff.c    \
          crypto/bio/b_print.c    \
          crypto/bio/b_dump.c     \
          crypto/bio/b_sock.c     \
          crypto/bio/bss_acpt.c   \
          crypto/bio/bf_nbio.c    \
          crypto/bio/bss_log.c    \
          crypto/bio/bss_bio.c    
endif
ifeq ($(OPENSSL_BUFFER),1)
SOURCES += crypto/buffer/buffer.c  \
          crypto/buffer/buf_err.c \
          crypto/buffer/buf_str.c
endif
ifeq ($(OPENSSL_COMP),1)
SOURCES += crypto/comp/comp_lib.c  \
          crypto/comp/c_rle.c     \
          crypto/comp/c_zlib.c    \
          crypto/comp/comp_err.c
endif
ifeq ($(OPENSSL_SSL),1)
SOURCES += ssl/bio_ssl.c           \
		  ssl/ssl_algs.c          \
          ssl/ssl_err.c           \
          ssl/ssl_sess.c          \
          ssl/ssl_ciph.c          \
          ssl/ssl_rsa.c           \
          ssl/ssl_cert.c          \
          ssl/ssl_lib.c           \
          ssl/ssl_txt.c           \
          ssl/ssl_asn1.c          \
          ssl/ssl_err2.c          \
          ssl/ssl_stat.c          \
          ssl/s3_both.c           \
          ssl/s3_meth.c           \
  		  ssl/s3_lib.c            \
  		  ssl/s3_enc.c            \
          ssl/s3_srvr.c           \
          ssl/s3_clnt.c           \
          ssl/s3_pkt.c            
ifeq ($(CONFIG_HTTPD_WITH_SSL), y)
SOURCES += ssl/s2_clnt.c ssl/s2_pkt.c ssl/s2_lib.c ssl/s2_meth.c ssl/s2_enc.c ssl/s2_srvr.c  ssl/s23_pkt.c  ssl/s23_clnt.c          \
          ssl/s23_srvr.c ssl/s23_lib.c ssl/s23_meth.c \
          ssl/t1_lib.c  ssl/t1_meth.c ssl/t1_clnt.c ssl/t1_srvr.c ssl/t1_enc.c	ssl/t1_reneg.c 
endif 

endif

#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)

all: depend  $(OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(COMMON_LIB) $(OBJS)


.PHONY:  depend

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<
depend:$(SOURCES) 
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)
clean:
	rm -f $(OBJS) *.o *~ 


