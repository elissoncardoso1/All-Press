import { Link } from 'react-router-dom';
import { Bell, User, Moon, Sun, Settings } from 'lucide-react';
import { Button } from '../ui/Button';
import { Badge } from '../ui/Badge';
import { useSystemStore } from '@/store/systemStore';
import { useUIStore } from '@/store/uiStore';

export const Header = () => {
  const notifications = useSystemStore((state) => state.notifications);
  const theme = useUIStore((state) => state.theme);
  const setTheme = useUIStore((state) => state.setTheme);
  
  const unreadCount = notifications.filter((n) => !n.read).length;

  const toggleTheme = () => {
    setTheme(theme === 'light' ? 'dark' : 'light');
  };

  return (
    <header className="sticky top-0 z-50 w-full border-b bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60">
      <div className="container flex h-16 items-center justify-between px-4">
        <div className="flex items-center gap-6">
          <Link to="/" className="flex items-center gap-2">
            <div className="flex h-8 w-8 items-center justify-center rounded-md bg-primary text-primary-foreground">
              <span className="text-lg font-bold">AP</span>
            </div>
            <span className="hidden text-lg font-bold sm:inline-block">All Press</span>
          </Link>
          
          <nav className="hidden md:flex items-center gap-6 text-sm">
            <Link to="/" className="transition-colors hover:text-foreground/80 text-foreground">
              Dashboard
            </Link>
            <Link to="/printers" className="transition-colors hover:text-foreground/80 text-foreground/60">
              Impressoras
            </Link>
            <Link to="/jobs" className="transition-colors hover:text-foreground/80 text-foreground/60">
              Jobs
            </Link>
            <Link to="/upload" className="transition-colors hover:text-foreground/80 text-foreground/60">
              Upload
            </Link>
            <Link to="/monitor" className="transition-colors hover:text-foreground/80 text-foreground/60">
              Monitor
            </Link>
          </nav>
        </div>

        <div className="flex items-center gap-2">
          <Button 
            variant="ghost" 
            size="icon" 
            onClick={toggleTheme}
            aria-label={theme === 'light' ? 'Alternar para tema escuro' : 'Alternar para tema claro'}
            title={theme === 'light' ? 'Tema escuro' : 'Tema claro'}
          >
            {theme === 'light' ? <Moon className="h-5 w-5" /> : <Sun className="h-5 w-5" />}
          </Button>

          <Button 
            variant="ghost" 
            size="icon" 
            className="relative"
            aria-label={`Notificações${unreadCount > 0 ? ` (${unreadCount} não lidas)` : ''}`}
            title="Notificações"
          >
            <Bell className="h-5 w-5" />
            {unreadCount > 0 && (
              <Badge
                variant="destructive"
                className="absolute -right-1 -top-1 h-5 w-5 rounded-full p-0 text-xs flex items-center justify-center"
                aria-label={`${unreadCount} notificações não lidas`}
              >
                {unreadCount}
              </Badge>
            )}
          </Button>

          <Button 
            variant="ghost" 
            size="icon"
            aria-label="Configurações"
            title="Configurações"
          >
            <Settings className="h-5 w-5" />
          </Button>

          <Button 
            variant="ghost" 
            size="icon"
            aria-label="Perfil do usuário"
            title="Perfil"
          >
            <User className="h-5 w-5" />
          </Button>
        </div>
      </div>
    </header>
  );
};
