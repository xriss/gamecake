--
-- Mysql helper functions
--

require"cgilua.cookies"

local sql=require("wetgenes.cgilua.mysql")


local type,ipairs=type,ipairs

local string=string

local wet_html=require("wetgenes.html")

local cgi = wetgenes.cgilua or require("wetgenes.cgilua")
local cfg = cfg


local cgilua=cgilua

local dbg=dbg or function(s) cgilua.errorlog(s) end

module("wetgenes.cgilua.session")



-----------------------------------------------------------------------------
--
-- log in using a session
-- if need is set to true then redirect to a login page as a login is needed
--
-- pass in url of current page if needed, (escaping is icky)
--
-----------------------------------------------------------------------------
function login(need)

local sess=cgilua.cookies.get(cfg.cookie_session)

local redirect="http://join.wetgenes."..cfg.tld.."/?redirect="..wet_html.url_esc(wet_html.url_esc(cgi.url_query))

user={}


	if sess then

		sess=string.gsub(sess, "[^0-9a-zA-Z]+", "" )


		local info=sql.execute([[SELECT
				u.alias, u.posts_ppg, u.time_zone, u.sig, u.last_visit, u.last_read, u.cat_collapse_status, u.users_opt,
				u.ignore_list, u.buddy_list, u.id, u.group_leader_list, u.email, u.login, u.sq, u.ban_expiry
				, u.avatar_loc , u.referer_id , s.time_sec , s.sys_id
			FROM ]]..cfg.mysql_prefix_fud..[[ses s
				INNER JOIN ]]..cfg.mysql_prefix_fud..[[users u ON u.id=(CASE WHEN s.user_id>2000000000 THEN 1 ELSE s.user_id END)
			WHERE ses_id=']]..sess..[[']])

		local tab=sql.named(info,1)

		if tab then -- session exists
		
			if cgi.ip==tab.sys_id then -- mild security fix, session is locked to ip
			
			
				user.fud=tab
				
				if tab.login=="XIX" or tab.login=="shi" then -- flag admin logins
					tab.admin=true
				end
			
--	dbg("success")
	
			else

				if need then
					cgi.redirect(redirect)
				end
--	dbg("fail bad ip "..cgi.ip..":"..tab.sys_id)

			end
			
			
		else -- user does not exist
		
--	dbg("fail bad sesson")
			
			if need then
				cgi.redirect(redirect)
			end
			
		end

	else
	
		if need then
			cgi.redirect(redirect)
		end
			
--	dbg("fail no session")
	
	end
	
	return user

end


