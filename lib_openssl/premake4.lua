
project "lib_openssl"
kind "StaticLib"
language "C"

files { "ssl/**.c" , "ssl/**.h" }
excludes { "ssl/ssl_task.c" , "ssl/ssltest.c" }


files { "crypto/err/**.c" }
files { "crypto/objects/**.c" }
files { "crypto/buffer/**.c" }
files { "crypto/stack/**.c" }
files { "crypto/comp/**.c" }
files { "crypto/pqueue/**.c" }
files { "crypto/dso/**.c" }
files { "crypto/ui/**.c" }
files { "crypto/ocsp/**.c" }
files { "crypto/txt_db/**.c" }
files { "crypto/cmac/**.c" }
files { "crypto/ts/**.c" }

for w in string.gmatch([[
	encode.c digest.c evp_enc.c evp_key.c evp_acnf.c evp_cnf.c 
	e_des.c e_bf.c e_idea.c e_des3.c e_camellia.c
	e_rc4.c e_aes.c names.c e_seed.c 
	e_xcbc_d.c e_rc2.c e_cast.c e_rc5.c 
	m_null.c m_md2.c m_md4.c m_md5.c m_sha.c m_sha1.c m_wp.c 
	m_dss.c m_dss1.c m_mdc2.c m_ripemd.c m_ecdsa.c
	p_open.c p_seal.c p_sign.c p_verify.c p_lib.c p_enc.c p_dec.c 
	bio_md.c bio_b64.c bio_enc.c evp_err.c e_null.c 
	c_all.c c_allc.c c_alld.c evp_lib.c bio_ok.c 
	evp_pkey.c evp_pbe.c p5_crpt.c p5_crpt2.c 
	e_old.c pmeth_lib.c pmeth_fn.c pmeth_gn.c m_sigver.c evp_fips.c	
	e_aes_cbc_hmac_sha1.c e_rc4_hmac_md5.c
]],"%S+") do
	files { "crypto/evp/"..w }
end


for w in string.gmatch( [[
	cryptlib.c mem.c mem_clr.c mem_dbg.c cversion.c ex_data.c cpt_err.c
	ebcdic.c uid.c o_time.c o_str.c o_dir.c o_fips.c o_init.c fips_ers.c
]],"%S+") do
	files { "crypto/"..w }
end

for w in string.gmatch( [[
	md_rand.c randfile.c rand_lib.c rand_err.c rand_egd.c 
	rand_win.c rand_unix.c rand_os2.c rand_nw.c
]],"%S+") do
	files { "crypto/rand/"..w }
end

for w in string.gmatch( [[
	x509_def.c x509_d2.c x509_r2x.c x509_cmp.c 
	x509_obj.c x509_req.c x509spki.c x509_vfy.c 
	x509_set.c x509cset.c x509rset.c x509_err.c 
	x509name.c x509_v3.c x509_ext.c x509_att.c 
	x509type.c x509_lu.c x_all.c x509_txt.c 
	x509_trs.c by_file.c by_dir.c x509_vpm.c
]],"%S+") do
	files { "crypto/x509/"..w }
end	

for w in string.gmatch( [[
	a_object.c a_bitstr.c a_utctm.c a_gentm.c a_time.c a_int.c a_octet.c 
	a_print.c a_type.c a_set.c a_dup.c a_d2i_fp.c a_i2d_fp.c 
	a_enum.c a_utf8.c a_sign.c a_digest.c a_verify.c a_mbstr.c a_strex.c 
	x_algor.c x_val.c x_pubkey.c x_sig.c x_req.c x_attrib.c x_bignum.c 
	x_long.c x_name.c x_x509.c x_x509a.c x_crl.c x_info.c x_spki.c nsseq.c 
	x_nx509.c d2i_pu.c d2i_pr.c i2d_pu.c i2d_pr.c
	t_req.c t_x509.c t_x509a.c t_crl.c t_pkey.c t_spki.c t_bitst.c 
	tasn_new.c tasn_fre.c tasn_enc.c tasn_dec.c tasn_utl.c tasn_typ.c 
	tasn_prn.c ameth_lib.c 
	f_int.c f_string.c n_pkey.c 
	f_enum.c x_pkey.c a_bool.c x_exten.c bio_asn1.c bio_ndef.c asn_mime.c 
	asn1_gen.c asn1_par.c asn1_lib.c asn1_err.c a_bytes.c a_strnid.c 
	evp_asn1.c asn_pack.c p5_pbe.c p5_pbev2.c p8_pkey.c asn_moid.c
]],"%S+") do
	files { "crypto/asn1/"..w }
end	
	
for w in string.gmatch( [[
	bio_lib.c bio_cb.c bio_err.c 
	bss_mem.c bss_null.c bss_fd.c 
	bss_file.c bss_sock.c bss_conn.c 
	bf_null.c bf_buff.c b_print.c b_dump.c 
	b_sock.c bss_acpt.c bf_nbio.c bss_log.c bss_bio.c 
	bss_dgram.c
]],"%S+") do
	files { "crypto/bio/"..w }
end	

for w in string.gmatch( [[
	rsa_eay.c rsa_gen.c rsa_lib.c rsa_sign.c rsa_saos.c rsa_err.c 
	rsa_pk1.c rsa_ssl.c rsa_none.c rsa_oaep.c rsa_chk.c rsa_null.c 
	rsa_pss.c rsa_x931.c rsa_asn1.c rsa_depr.c rsa_ameth.c rsa_prn.c 
	rsa_pmeth.c rsa_crpt.c
]],"%S+") do
	files { "crypto/rsa/"..w }
end	

for w in string.gmatch( [[
	bn_add.c bn_div.c bn_exp.c bn_lib.c bn_ctx.c bn_mul.c bn_mod.c 
	bn_print.c bn_rand.c bn_shift.c bn_word.c bn_blind.c 
	bn_kron.c bn_sqrt.c bn_gcd.c bn_prime.c bn_err.c bn_sqr.c bn_asm.c 
	bn_recp.c bn_mont.c bn_mpi.c bn_exp2.c bn_gf2m.c bn_nist.c 
	bn_depr.c bn_const.c bn_x931p.c
]],"%S+") do
	files { "crypto/bn/"..w }
end		

for w in string.gmatch( [[
	v3_bcons.c v3_bitst.c v3_conf.c v3_extku.c v3_ia5.c v3_lib.c 
	v3_prn.c v3_utl.c v3err.c v3_genn.c v3_alt.c v3_skey.c v3_akey.c v3_pku.c 
	v3_int.c v3_enum.c v3_sxnet.c v3_cpols.c v3_crld.c v3_purp.c v3_info.c 
	v3_ocsp.c v3_akeya.c v3_pmaps.c v3_pcons.c v3_ncons.c v3_pcia.c v3_pci.c 
	pcy_cache.c pcy_node.c pcy_data.c pcy_map.c pcy_tree.c pcy_lib.c 
	v3_asid.c v3_addr.c
]],"%S+") do
	files { "crypto/x509v3/"..w }
end		

for w in string.gmatch( [[
	conf_err.c conf_lib.c conf_api.c conf_def.c conf_mod.c 
	conf_mall.c conf_sap.c
]],"%S+") do
	files { "crypto/conf/"..w }
end		

for w in string.gmatch( [[
	lhash.c lh_stats.c
]],"%S+") do
	files { "crypto/lhash/"..w }
end		

for w in string.gmatch( [[
	sha_dgst.c sha1dgst.c sha_one.c sha1_one.c sha256.c sha512.c
]],"%S+") do
	files { "crypto/sha/"..w }
end		

for w in string.gmatch( [[
	md5_dgst.c md5_one.c
]],"%S+") do
	files { "crypto/md5/"..w }
end		

for w in string.gmatch( [[
	pem_sign.c pem_seal.c pem_info.c pem_lib.c pem_all.c pem_err.c
	pem_x509.c pem_xaux.c pem_oth.c pem_pk8.c pem_pkey.c pvkfmt.c
]],"%S+") do
	files { "crypto/pem/"..w }
end

for w in string.gmatch( [[
	pk7_asn1.c pk7_lib.c pkcs7err.c pk7_doit.c pk7_smime.c pk7_attr.c \
	pk7_mime.c bio_pk7.c
]],"%S+") do
	files { "crypto/pkcs7/"..w }
end	
	
for w in string.gmatch( [[
	p12_add.c p12_asn.c p12_attr.c p12_crpt.c p12_crt.c p12_decr.c 
	p12_init.c p12_key.c p12_kiss.c p12_mutl.c
	p12_utl.c p12_npas.c pk12err.c p12_p8d.c p12_p8e.c
]],"%S+") do
	files { "crypto/pkcs12/"..w }
end


for w in string.gmatch( [[
	ec_lib.c ecp_smpl.c ecp_mont.c ecp_nist.c ec_cvt.c ec_mult.c
	ec_err.c ec_curve.c ec_check.c ec_print.c ec_asn1.c ec_key.c
	ec2_smpl.c ec2_mult.c ec_ameth.c ec_pmeth.c eck_prn.c 
	ecp_nistp224.c ecp_nistp256.c ecp_nistp521.c ecp_nistputil.c 
	ecp_oct.c ec2_oct.c ec_oct.c
]],"%S+") do
	files { "crypto/ec/"..w }
end
	
for w in string.gmatch( [[
	dh_asn1.c dh_gen.c dh_key.c dh_lib.c dh_check.c dh_err.c dh_depr.c 
	dh_ameth.c dh_pmeth.c dh_prn.c	
]],"%S+") do
	files { "crypto/dh/"..w }
end

for w in string.gmatch( [[
	hmac.c hm_ameth.c hm_pmeth.c
]],"%S+") do
	files { "crypto/hmac/"..w }
end

for w in string.gmatch( [[
	eng_err.c eng_lib.c eng_list.c eng_init.c eng_ctrl.c 
	eng_table.c eng_pkey.c eng_fat.c eng_all.c 
	tb_rsa.c tb_dsa.c tb_ecdsa.c tb_dh.c tb_ecdh.c tb_rand.c tb_store.c 
	tb_cipher.c tb_digest.c tb_pkmeth.c tb_asnmth.c 
	eng_openssl.c eng_cnf.c eng_dyn.c eng_cryptodev.c 
	eng_rsax.c eng_rdrand.c
]],"%S+") do
	files { "crypto/engine/"..w }
end

for w in string.gmatch( [[
	ecs_lib.c ecs_asn1.c ecs_ossl.c ecs_sign.c ecs_vrf.c ecs_err.c
]],"%S+") do
	files { "crypto/ecdsa/"..w }
end

for w in string.gmatch( [[
	ech_lib.c ech_ossl.c ech_key.c ech_err.c
]],"%S+") do
	files { "crypto/ecdh/"..w }
end

for w in string.gmatch( [[
	dsa_gen.c dsa_key.c dsa_lib.c dsa_asn1.c dsa_vrf.c dsa_sign.c 
	dsa_err.c dsa_ossl.c dsa_depr.c dsa_ameth.c dsa_pmeth.c dsa_prn.c
]],"%S+") do
	files { "crypto/dsa/"..w }
end

for w in string.gmatch( [[
	cms_lib.c cms_asn1.c cms_att.c cms_io.c cms_smime.c cms_err.c \
	cms_sd.c cms_dd.c cms_cd.c cms_env.c cms_enc.c cms_ess.c \
	cms_pwri.c
]],"%S+") do
	files { "crypto/cms/"..w }
end

for w in string.gmatch( [[
	aes_core.c aes_misc.c aes_ecb.c aes_cbc.c aes_cfb.c aes_ofb.c
    aes_ctr.c aes_ige.c aes_wrap.c
]],"%S+") do
	files { "crypto/aes/"..w }
end

for w in string.gmatch( [[
	rc2_ecb.c rc2_skey.c rc2_cbc.c rc2cfb64.c rc2ofb64.c
]],"%S+") do
	files { "crypto/rc2/"..w }
end

for w in string.gmatch( [[
	rc4_skey.c rc4_enc.c rc4_utl.c
]],"%S+") do
	files { "crypto/rc4/"..w }
end

for w in string.gmatch( [[
	cbc128.c ctr128.c cts128.c cfb128.c ofb128.c gcm128.c 
	ccm128.c xts128.c
]],"%S+") do
	files { "crypto/modes/"..w }
end

for w in string.gmatch( [[
	seed.c seed_ecb.c seed_cbc.c seed_cfb.c seed_ofb.c
]],"%S+") do
	files { "crypto/seed/"..w }
end

for w in string.gmatch( [[
	camellia.c cmll_misc.c cmll_ecb.c cmll_cbc.c cmll_ofb.c 
	   cmll_cfb.c cmll_ctr.c cmll_utl.c
]],"%S+") do
	files { "crypto/camellia/"..w }
end

for w in string.gmatch( [[
	cbc_cksm.c cbc_enc.c  cfb64enc.c cfb_enc.c  
	ecb3_enc.c ecb_enc.c  enc_read.c enc_writ.c 
	fcrypt.c ofb64enc.c ofb_enc.c  pcbc_enc.c 
	qud_cksm.c rand_key.c rpc_enc.c  set_key.c 
	des_enc.c fcrypt_b.c 
	xcbc_enc.c 
	str2key.c  cfb64ede.c ofb64ede.c ede_cbcm_enc.c des_old.c des_old2.c 
	read2pwd.c
]],"%S+") do
	files { "crypto/des/"..w }
end

for w in string.gmatch( [[
	i_cbc.c i_cfb64.c i_ofb64.c i_ecb.c i_skey.c
]],"%S+") do
	files { "crypto/idea/"..w }
end

for w in string.gmatch( [[
	srp_lib.c srp_vfy.c	
]],"%S+") do
	files { "crypto/srp/"..w }
end

includedirs { ".", "include" , "crypto" , "crypto/asn1" , "crypto/modes" , "crypto/evp" }


KIND{}

